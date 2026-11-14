# SPDX-License-Identifier: Apache-2.0
"""Model data checks: every model resolves, and the transcription matches the
control tables of the ROBOTIS e-Manual (tests/models/emanual_x430.json) except
for the documented deviations of the emulator."""

import json
import os
import pathlib
import re
import sys
import tempfile
import unittest

ROOT = pathlib.Path(os.environ.get("X430_ROOT", pathlib.Path(__file__).resolve().parents[2]))
sys.path.insert(0, str(ROOT / "tools"))

import x430_gen_model as gen  # noqa: E402

MODELS = ROOT / "models"
SNAPSHOT = json.loads((ROOT / "tests" / "models" / "emanual_x430.json").read_text(encoding="utf-8"))

# Intentional differences from the e-Manual tables, (address, field) -> reason.
DEVIATIONS = {
    (12, "range"): "Secondary ID accepts 253..255 (they disable the function)",
    (13, "range"): "Protocol Type: only Protocol 2.0 is emulated",
    (13, "initial"): "Protocol Type: XW pages show 1, the emulator starts in Protocol 2.0",
    (98, "range"): "Bus Watchdog accepts 0 (disable / clear error)",
}
MODEL_DEVIATIONS = {
    ("xw430_t200", 0, "initial"): "Model Number 1280 (the e-Manual table repeats 1270 of XW430-T333)",
}


class ModelData(unittest.TestCase):
    def test_every_model_resolves_and_renders(self):
        keys = gen.list_models(MODELS)
        self.assertEqual(len(keys), 17)
        with tempfile.TemporaryDirectory() as out:
            for key in keys:
                with self.subTest(model=key):
                    self.assertEqual(gen.main(["--models-dir", str(MODELS), "--model", key, "--out-dir", out]), 0)
                    text = pathlib.Path(out, "x430_model_table.c").read_text()
                    self.assertIn("x430_model_active", text)

    def test_kconfig_lists_every_model(self):
        kconfig = (ROOT / "Kconfig.x430").read_text()
        for key in gen.list_models(MODELS):
            with self.subTest(model=key):
                self.assertIn(f'default "{key}" if X430_MODEL_{key.upper()}', kconfig)

    def test_matches_emanual(self):
        for key in gen.list_models(MODELS):
            model = gen.resolve(MODELS, key)
            rows = {r["addr"]: r for r in SNAPSHOT["models"][key.replace("_", "-")]}
            regs = {r.address: r for r in model.registers}
            for address, row in rows.items():
                if "Indirect" in row["name"]:
                    continue
                with self.subTest(model=key, address=address, name=row["name"]):
                    self.assertIn(address, regs)
                    reg = regs[address]
                    self.assertEqual(reg.size, row["size"])
                    self.assertEqual(reg.access, row["access"].lower())
                    self.assertEqual(reg.area, row["area"].lower())
                    initial = row["init"].split()[0]
                    if re.fullmatch(r"-?\d+", initial) and address != 7:
                        if (address, "initial") not in DEVIATIONS and (key, address, "initial") not in MODEL_DEVIATIONS:
                            self.assertEqual(reg.initial, int(initial))
                    found = re.match(r"^(-?\d+) ~ (-?\d+)", row["range"])
                    if found and reg.check == "range" and (address, "range") not in DEVIATIONS:
                        self.assertEqual((reg.minimum, reg.maximum), (int(found.group(1)), int(found.group(2))))
            for address, reg in regs.items():
                if not reg.name.startswith("Indirect"):
                    with self.subTest(model=key, extra=reg.name):
                        self.assertIn(address, rows)

    def test_bad_model_is_reported(self):
        self.assertEqual(gen.main(["--models-dir", str(MODELS), "--model", "xx999", "--out-dir", "/tmp"]), 1)


    def _overrides_file(self, directory, text):
        path = pathlib.Path(directory) / "overrides.yaml"
        path.write_text(text, encoding="utf-8")
        return path

    def test_application_overrides(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = self._overrides_file(tmp, "overrides:\n"
                                        "  Max Voltage Limit: {max: 300, initial: 280}\n"
                                        "  Min Voltage Limit: {max: 300}\n")
            model = gen.resolve(MODELS, "xm430_w350", extra_overrides=gen.load_overrides(path))
            regs = {r.name: r for r in model.registers}
            self.assertEqual((regs["Max Voltage Limit"].maximum, regs["Max Voltage Limit"].initial),
                             (300, 280))
            self.assertEqual(regs["Min Voltage Limit"].maximum, 300)
            self.assertEqual(regs["Min Voltage Limit"].initial, 95)       # not overridden
            out = pathlib.Path(tmp) / "gen"
            self.assertEqual(gen.main(["--models-dir", str(MODELS), "--model", "xm430_w350",
                                       "--out-dir", str(out), "--overrides", str(path)]), 0)
            self.assertIn("300", (out / "x430_model_table.c").read_text(encoding="utf-8"))
        self.assertEqual(gen.load_overrides(None), {})

    def test_bad_application_overrides_are_reported(self):
        with tempfile.TemporaryDirectory() as tmp:
            unknown = self._overrides_file(tmp, "overrides:\n  No Such Register: {max: 1}\n")
            with self.assertRaises(gen.ModelError):
                gen.resolve(MODELS, "xm430_w350", extra_overrides=gen.load_overrides(unknown))
            outside = self._overrides_file(tmp, "overrides:\n  Max Voltage Limit: {initial: 200}\n")
            with self.assertRaises(gen.ModelError):
                gen.resolve(MODELS, "xm430_w350", extra_overrides=gen.load_overrides(outside))
            not_mapping = self._overrides_file(tmp, "overrides:\n  Max Voltage Limit: 3\n")
            with self.assertRaises(gen.ModelError):
                gen.load_overrides(not_mapping)


if __name__ == "__main__":
    unittest.main()
