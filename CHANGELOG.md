# Changelog

All notable changes to this project are documented here. Versions follow
[Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added
- Device-side emulation of all 17 DYNAMIXEL X430 models (Protocol 2.0), one
  model per build, Control Tables generated from `models/` (YAML, transcribed
  from the ROBOTIS e-Manual and checked against it by `tests/models`).
- Control Table rules: access rights, ranges and limit registers, EEPROM lock,
  Indirect Address/Data, REG_WRITE/ACTION, Secondary ID, Status Return Level,
  Return Delay Time, Bus Watchdog, hardware error shutdown and alert bit,
  Startup Configuration, Drive Mode "torque on by goal update".
- Instructions: PING, READ, WRITE, REG_WRITE, ACTION, FACTORY_RESET, REBOOT,
  CLEAR, BACKUP, SYNC_READ/WRITE, BULK_READ/WRITE, FAST_SYNC_READ,
  FAST_BULK_READ.
- Application hooks (write, read, event, persistent storage) and an optional
  platform port (lock, millisecond clock, delay).
- Simulator backend (`x430_sim`) and the POSIX host example `x430_host_sim`.
- Kconfig (Kconfiglib) and plain-CMake configuration; DXL2-C v1.0.0 through
  FetchContent.
- GoogleTest suite, model data checks, end-to-end check with the ROBOTIS
  DynamixelSDK.
