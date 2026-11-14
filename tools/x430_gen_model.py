#!/usr/bin/env python3
################################################################################
# sfs-dynamixel-x430: DYNAMIXEL X430 device emulator
#
# Project:     sfs-dynamixel-x430 - DYNAMIXEL X430 Control Table Emulator
# Description: Generates the C model descriptor (Control Table) of one model
#              from models/<family>.yaml and models/<family>/<model>.yaml.
#
# Author:      Anton Vynohradov
# Email:       avynohradov@systemfromscratch.com
# License:     Apache License 2.0
# Copyright:   (c) 2026 Anton Vynohradov
#
# Usage:
#   x430_gen_model.py --models-dir models --model xm430_w350 --out-dir build/gen
#   x430_gen_model.py --models-dir models --list
#   x430_gen_model.py --models-dir models --dump xm430_w350   (resolved table)
#   x430_gen_model.py ... --overrides product.yaml   (application overrides, see resolve())
#
# Outputs (in --out-dir):
#   x430_model_info.h   compile-time facts of the selected model
#   x430_model_table.c  const struct x430_model x430_model_active
#
# SPDX-License-Identifier: Apache-2.0
################################################################################

"""Generate the Control Table descriptor of one DYNAMIXEL X430 model."""

from __future__ import annotations

import argparse
import pathlib
import re
import sys
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional

try:
    import yaml
except ImportError:  # the built-in reader below handles the model files
    yaml = None


# ------------------------------------------------------------------------------
# YAML subset reader (used when PyYAML is not installed)
#
# The model files use a small part of YAML only: comments, block mappings
# indented by spaces, block lists whose items are flow mappings, flow mappings
# {k: v, ...}, flow lists [a, b] and plain or quoted scalars (decimal and hex
# integers, true/false, null, strings). tests/models checks that this reader
# returns the same data as PyYAML for every file in models/.
# ------------------------------------------------------------------------------

def _scalar(text: str) -> Any:
    text = text.strip()
    if len(text) >= 2 and text[0] == text[-1] and text[0] in "'\"":
        return text[1:-1]
    lowered = text.lower()
    if lowered in ("true", "yes", "on"):
        return True
    if lowered in ("false", "no", "off"):
        return False
    if lowered in ("", "~", "null"):
        return None
    if re.fullmatch(r"[-+]?\d+", text):
        return int(text)
    if re.fullmatch(r"[-+]?0x[0-9a-fA-F]+", text):
        return int(text, 16)
    return text


def _split_flow(text: str) -> List[str]:
    """Items of a flow collection body, split at top-level commas."""
    items, depth, start, quote = [], 0, 0, ""
    for index, char in enumerate(text):
        if quote:
            quote = "" if char == quote else quote
        elif char in "'\"":
            quote = char
        elif char in "[{":
            depth += 1
        elif char in "]}":
            depth -= 1
        elif char == "," and depth == 0:
            items.append(text[start:index])
            start = index + 1
    items.append(text[start:])
    return [item for item in items if item.strip()]


def _flow(text: str) -> Any:
    text = text.strip()
    if text.startswith("{") and text.endswith("}"):
        result = {}
        for item in _split_flow(text[1:-1]):
            key, _, value = item.partition(":")
            result[key.strip()] = _flow(value)
        return result
    if text.startswith("[") and text.endswith("]"):
        return [_flow(item) for item in _split_flow(text[1:-1])]
    return _scalar(text)


def _strip_comment(line: str) -> str:
    quote = ""
    for index, char in enumerate(line):
        if quote:
            quote = "" if char == quote else quote
        elif char in "'\"":
            quote = char
        elif char == "#" and (index == 0 or line[index - 1] in " \t"):
            return line[:index]
    return line


def read_yaml_subset(text: str) -> Any:
    """Parse the YAML subset of the model files (see above)."""
    lines = []
    for raw in text.splitlines():
        line = _strip_comment(raw).rstrip()
        if line.strip():
            lines.append((len(line) - len(line.lstrip(" ")), line.strip()))

    def block(position: int, indent: int):
        if lines[position][1].startswith("- "):
            items = []
            while position < len(lines) and lines[position][0] == indent \
                    and lines[position][1].startswith("- "):
                items.append(_flow(lines[position][1][2:]))
                position += 1
            return items, position
        mapping: Dict[str, Any] = {}
        while position < len(lines) and lines[position][0] == indent:
            key, _, value = lines[position][1].partition(":")
            position += 1
            if value.strip():
                mapping[key.strip()] = _flow(value)
            elif position < len(lines) and lines[position][0] > indent:
                mapping[key.strip()], position = block(position, lines[position][0])
            elif position < len(lines) and lines[position][0] == indent \
                    and lines[position][1].startswith("- "):
                mapping[key.strip()], position = block(position, indent)
            else:
                mapping[key.strip()] = None
        return mapping, position

    if not lines:
        return None
    data, position = block(0, lines[0][0])
    if position != len(lines):
        raise ValueError(f"unexpected indentation: {lines[position][1]!r}")
    return data

CHECKS = {
    "none": "X430_CHECK_NONE",
    "range": "X430_CHECK_RANGE",
    "mask": "X430_CHECK_MASK",
    "opmode": "X430_CHECK_OPMODE",
    "symmetric": "X430_CHECK_SYMMETRIC",
    "goal_position": "X430_CHECK_GOAL_POSITION",
    "indirect": "X430_CHECK_INDIRECT",
}

VALID_SIZES = (1, 2, 4)


@dataclass
class Register:
    """One Control Table item after the model overrides are applied."""

    name: str
    address: int
    size: int
    area: str
    access: str
    initial: int
    check: str
    minimum: int = 0
    maximum: int = 0
    mask: int = 0
    ref: int = 0
    signed: bool = False
    goal: bool = False

    def flags(self) -> List[str]:
        result = []
        if self.access == "rw":
            result.append("X430_REG_WRITABLE")
        if self.area == "eeprom":
            result.append("X430_REG_EEPROM")
        if self.signed:
            result.append("X430_REG_SIGNED")
        if self.goal:
            result.append("X430_REG_GOAL")
        return result


@dataclass
class Model:
    """Resolved model: identity, capabilities and Control Table."""

    key: str
    name: str
    number: int
    family: str
    variant: str
    table_size: int
    operating_modes: List[int]
    current_unit_ua: int
    axes: int
    registers: List[Register] = field(default_factory=list)
    indirect: List[Dict[str, int]] = field(default_factory=list)
    notes: List[str] = field(default_factory=list)


class ModelError(Exception):
    """Inconsistent model data."""


def _load_yaml(path: pathlib.Path) -> dict:
    text = path.read_text(encoding="utf-8")
    try:
        data = yaml.safe_load(text) if yaml is not None else read_yaml_subset(text)
    except ValueError as error:
        raise ModelError(f"{path}: {error}") from error
    if not isinstance(data, dict):
        raise ModelError(f"{path}: expected a mapping")
    return data


def list_models(models_dir: pathlib.Path) -> List[str]:
    """Model keys available in models_dir (every family sub-directory)."""
    keys = []
    for family_file in sorted(models_dir.glob("*_family.yaml")):
        family = family_file.name[: -len("_family.yaml")]
        keys.extend(sorted(p.stem for p in (models_dir / family).glob("*.yaml")))
    return keys


def _find_model_file(models_dir: pathlib.Path, key: str) -> pathlib.Path:
    for family_file in sorted(models_dir.glob("*_family.yaml")):
        family = family_file.name[: -len("_family.yaml")]
        candidate = models_dir / family / f"{key}.yaml"
        if candidate.is_file():
            return candidate
    raise ModelError(f"unknown model '{key}' (available: {', '.join(list_models(models_dir))})")


def load_overrides(path: Optional[pathlib.Path]) -> Dict[str, dict]:
    """Application overrides: a YAML file with an `overrides:` mapping, as in a model file.

    Example (a product whose supply is above the XM430 voltage range):

        overrides:
          Max Voltage Limit: {max: 300}
          Min Voltage Limit: {max: 300}
    """
    if path is None:
        return {}
    data = _load_yaml(path) or {}
    overrides = data.get("overrides", {}) or {}
    if not isinstance(overrides, dict) or not all(isinstance(v, dict) for v in overrides.values()):
        raise ModelError(f"{path}: 'overrides' must map register names to field mappings")
    return overrides


def resolve(models_dir: pathlib.Path, key: str, firmware_placeholder: int = 0,
            extra_overrides: Optional[Dict[str, dict]] = None) -> Model:
    """Merge the family table with the model file and validate the result.

    `extra_overrides` (load_overrides()) apply after the overrides of the model file:
    an application changes initial values or ranges without editing models/.
    """
    model_file = _find_model_file(models_dir, key)
    spec = _load_yaml(model_file)
    family_file = models_dir / f"{spec['family']}_family.yaml"
    family = _load_yaml(family_file)
    extra = extra_overrides or {}
    unknown_extra = set(extra)

    variant = spec["variant"]
    removed = set(spec.get("remove", []))
    overrides: Dict[str, dict] = spec.get("overrides", {}) or {}
    unknown = set(overrides) | removed

    model = Model(
        key=spec["model"],
        name=spec["name"],
        number=int(spec["model_number"]),
        family=spec["family"],
        variant=variant,
        table_size=int(family["table_size"]),
        operating_modes=[int(m) for m in spec["operating_modes"]],
        current_unit_ua=int(spec.get("current_unit_ua", 0)),
        axes=int(spec.get("axes", 1)),
        indirect=list(family.get("indirect", [])),
        notes=list(spec.get("notes", []) or []),
    )
    if model.key != key:
        raise ModelError(f"{model_file}: 'model: {model.key}' does not match the file name")

    for item in family["registers"]:
        if item.get("variant", variant) != variant or item["name"] in removed:
            unknown.discard(item["name"])
            continue
        item = dict(item)
        item.update(overrides.get(item["name"], {}))
        item.update(extra.get(item["name"], {}))
        unknown.discard(item["name"])
        unknown_extra.discard(item["name"])
        initial = item["initial"]
        if initial == "model":
            initial = model.number
        elif initial == "firmware":
            initial = firmware_placeholder
        model.registers.append(
            Register(
                name=item["name"],
                address=int(item["address"]),
                size=int(item["size"]),
                area=item["area"],
                access=item["access"],
                initial=int(initial),
                check=item["check"],
                minimum=int(item.get("min", 0)),
                maximum=int(item.get("max", 0)),
                mask=int(item.get("mask", 0)),
                ref=int(item.get("ref", 0)),
                signed=bool(item.get("signed", False)),
                goal=bool(item.get("goal", False)),
            )
        )
    if unknown:
        raise ModelError(f"{model_file}: overrides/remove name unknown registers: {sorted(unknown)}")
    if unknown_extra:
        raise ModelError(f"application overrides name unknown registers: {sorted(unknown_extra)}")

    # Indirect Address registers (2 bytes, RAM, RW) are generated from the blocks.
    for block in model.indirect:
        for index in range(int(block["count"])):
            model.registers.append(
                Register(
                    name=f"Indirect Address {index + 1}",
                    address=int(block["address"]) + 2 * index,
                    size=2,
                    area="ram",
                    access="rw",
                    initial=int(block["data"]) + index,
                    check="indirect",
                    minimum=64,
                    maximum=model.table_size - 1,
                )
            )
    model.registers.sort(key=lambda r: r.address)
    _validate(model)
    for reg in model.registers:
        if reg.name in extra and reg.check == "range" and not (
                reg.minimum <= reg.initial <= reg.maximum):
            raise ModelError(f"application overrides: {reg.name}: initial {reg.initial} "
                             f"outside {reg.minimum}..{reg.maximum}")
    return model


def _validate(model: Model) -> None:
    used: Dict[int, str] = {}
    by_address = {r.address: r for r in model.registers}
    for reg in model.registers:
        if reg.size not in VALID_SIZES:
            raise ModelError(f"{model.key}: {reg.name}: size {reg.size}")
        if reg.check not in CHECKS:
            raise ModelError(f"{model.key}: {reg.name}: unknown check '{reg.check}'")
        if reg.access not in ("r", "rw") or reg.area not in ("eeprom", "ram"):
            raise ModelError(f"{model.key}: {reg.name}: bad access/area")
        if reg.area == "eeprom" and reg.address >= 64:
            raise ModelError(f"{model.key}: {reg.name}: EEPROM register above address 63")
        if reg.address + reg.size > model.table_size:
            raise ModelError(f"{model.key}: {reg.name}: beyond the table size")
        if reg.access == "r" and reg.check != "none":
            raise ModelError(f"{model.key}: {reg.name}: read-only register with a write check")
        if reg.access == "rw" and reg.check == "none":
            raise ModelError(f"{model.key}: {reg.name}: writable register without a check")
        if reg.check == "range" and reg.minimum > reg.maximum:
            raise ModelError(f"{model.key}: {reg.name}: min > max")
        if reg.check == "symmetric":
            if reg.ref not in by_address:
                raise ModelError(f"{model.key}: {reg.name}: ref {reg.ref} is not a register")
        for offset in range(reg.size):
            if reg.address + offset in used:
                raise ModelError(
                    f"{model.key}: {reg.name} overlaps {used[reg.address + offset]}"
                )
            used[reg.address + offset] = reg.name
    for block in model.indirect:
        for index in range(int(block["count"])):
            if int(block["data"]) + index in used:
                raise ModelError(f"{model.key}: indirect data overlaps a register")
    if len(model.registers) > 254:
        raise ModelError(f"{model.key}: more than 254 registers (map uses uint8_t)")
    if any(m < 0 or m > 31 for m in model.operating_modes):
        raise ModelError(f"{model.key}: operating mode outside 0..31")
    if model.axes not in (1, 2):
        raise ModelError(f"{model.key}: axes must be 1 or 2")


def _c_int(value: int) -> str:
    if value < 0:
        return f"({value})"
    return f"{value}"


def render_info_header(model: Model) -> str:
    names = {r.name for r in model.registers}
    guard = "X430_MODEL_INFO_H"
    macro = "X430_MODEL_IS_" + model.key.upper()
    lines = [
        "/* Generated by tools/x430_gen_model.py - do not edit. */",
        "/* SPDX-License-Identifier: Apache-2.0 */",
        "",
        "/**",
        f" * @file x430_model_info.h",
        f" * @brief Compile-time facts of the emulated model: {model.name}.",
        " */",
        "",
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        f'#define X430_MODEL_NAME "{model.name}"',
        f'#define X430_MODEL_KEY "{model.key}"',
        f"#define {macro} 1",
        f"/** Model Number from the ROBOTIS documentation (see CONFIG_X430_MODEL_NUMBER_OVERRIDE). */",
        f"#define X430_MODEL_NUMBER_DEFAULT {model.number}U",
        f"#define X430_TABLE_SIZE {model.table_size}U",
        f"#define X430_MODEL_AXES {model.axes}U",
        f"#define X430_MODEL_HAS_CURRENT {1 if model.variant == 'current' else 0}",
        f"#define X430_MODEL_HAS_LED {1 if 'LED' in names else 0}",
        f"/** Current unit in uA (0: no current control, Present Load instead). */",
        f"#define X430_MODEL_CURRENT_UNIT_UA {model.current_unit_ua}U",
        "",
        f"#endif /* {guard} */",
        "",
    ]
    return "\n".join(lines)


def render_table_source(model: Model) -> str:
    mode_mask = 0
    for mode in model.operating_modes:
        mode_mask |= 1 << mode
    reg_map = [0] * model.table_size
    for index, reg in enumerate(model.registers):
        for offset in range(reg.size):
            reg_map[reg.address + offset] = index + 1

    out = [
        "/* Generated by tools/x430_gen_model.py - do not edit. */",
        "/* SPDX-License-Identifier: Apache-2.0 */",
        "",
        "/**",
        " * @file x430_model_table.c",
        f" * @brief Control Table of {model.name} (Model Number {model.number}).",
        " *",
        " * @details Source: ROBOTIS e-Manual, transcribed in models/"
        f"{model.family}/{model.key}.yaml.",
    ]
    for note in model.notes:
        out.append(f" *          Note: {note}")
    out += [
        " */",
        "",
        '#include "x430_model.h"',
        "",
        "/* clang-format off */",
        "",
        f"static const struct x430_reg x430_registers[{len(model.registers)}U] = {{",
    ]
    for reg in model.registers:
        flags = " | ".join(reg.flags()) or "0U"
        if reg.check == "mask":
            lo, hi = 0, reg.mask
        else:
            lo, hi = reg.minimum, reg.maximum
        out.append(
            f"    {{{reg.address}U, {reg.size}U, (uint8_t) ({flags}), {CHECKS[reg.check]}, "
            f"{reg.ref}U, {_c_int(reg.initial)}, {_c_int(lo)}, {_c_int(hi)}}}, /* {reg.name} */"
        )
    out.append("};")
    out.append("")
    out.append(f"static const struct x430_indirect_block x430_indirect[{max(len(model.indirect), 1)}U] = {{")
    if model.indirect:
        for block in model.indirect:
            out.append(f"    {{{block['address']}U, {block['data']}U, {block['count']}U}},")
    else:
        out.append("    {0U, 0U, 0U},")
    out.append("};")
    out.append("")
    out.append(
        "/* Register index + 1 for every address (0 = reserved address). */"
    )
    out.append(f"static const uint8_t x430_register_map[{model.table_size}U] = {{")
    for start in range(0, model.table_size, 16):
        chunk = ", ".join(f"{v}U" for v in reg_map[start : start + 16])
        out.append(f"    {chunk}, /* {start} */")
    out.append("};")
    out += [
        "",
        "const struct x430_model x430_model_active = {",
        f'    .name = "{model.name}",',
        f"    .model_number = {model.number}U,",
        f"    .table_size = {model.table_size}U,",
        "    .registers = x430_registers,",
        f"    .register_count = {len(model.registers)}U,",
        "    .register_map = x430_register_map,",
        "    .indirect = x430_indirect,",
        f"    .indirect_count = {len(model.indirect)}U,",
        f"    .operating_modes = 0x{mode_mask:08X}UL,",
        f"    .current_unit_ua = {model.current_unit_ua}U,",
        f"    .axes = {model.axes}U,",
        "};",
        "",
        "/* clang-format on */",
        "",
    ]
    return "\n".join(out)


def _write_if_changed(path: pathlib.Path, text: str) -> None:
    if path.is_file() and path.read_text(encoding="utf-8") == text:
        return
    path.write_text(text, encoding="utf-8")


def main(argv: Optional[List[str]] = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--models-dir", required=True, type=pathlib.Path)
    parser.add_argument("--model", help="model key, e.g. xm430_w350")
    parser.add_argument("--out-dir", type=pathlib.Path)
    parser.add_argument("--list", action="store_true", help="print the available model keys")
    parser.add_argument("--dump", metavar="MODEL", help="print the resolved Control Table")
    parser.add_argument("--overrides", type=pathlib.Path,
                        help="YAML file with application overrides (overrides: {name: {...}})")
    args = parser.parse_args(argv)

    try:
        if args.list:
            print("\n".join(list_models(args.models_dir)))
            return 0
        if args.dump:
            model = resolve(args.models_dir, args.dump,
                            extra_overrides=load_overrides(args.overrides))
            print(f"{model.name} ({model.number}), variant {model.variant}, modes {model.operating_modes}")
            for reg in model.registers:
                print(
                    f"{reg.address:4d} {reg.size} {reg.area:6s} {reg.access:2s} "
                    f"{reg.name:28s} init={reg.initial} check={reg.check} "
                    f"[{reg.minimum}..{reg.maximum}] mask=0x{reg.mask:X} ref={reg.ref}"
                )
            return 0
        if not args.model or not args.out_dir:
            parser.error("--model and --out-dir are required")
        model = resolve(args.models_dir, args.model,
                        extra_overrides=load_overrides(args.overrides))
        args.out_dir.mkdir(parents=True, exist_ok=True)
        _write_if_changed(args.out_dir / "x430_model_info.h", render_info_header(model))
        _write_if_changed(args.out_dir / "x430_model_table.c", render_table_source(model))
    except (ModelError, KeyError) as error:
        print(f"x430_gen_model.py: error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
