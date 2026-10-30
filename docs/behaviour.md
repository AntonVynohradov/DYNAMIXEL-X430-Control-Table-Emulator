# Behaviour

What the emulator does, so you can predict what a host sees.

## Addressing

- A packet to an ID is executed by the device with that **ID**; only it answers.
- Devices whose **Secondary ID** (12) equals the packet ID execute it silently. They
  refuse EEPROM writes and FACTORY_RESET. Values 253..255 disable the Secondary ID.
- **Broadcast** (0xFE): PING is answered by every device in ascending ID order; WRITE,
  REG_WRITE, ACTION, REBOOT, CLEAR, BACKUP and FACTORY_RESET (except option 0xFF) are
  executed silently; READ is ignored.
- Status packets on the line (from other devices) are ignored.

## Status packets

- **Status Return Level** (68): 0 = PING only, 1 = PING and reads, 2 = all. The level
  written by an instruction already applies to that instruction's answer.
- **Return Delay Time** (9) × 2 µs is waited through `x430_port::delay_us` before each
  answer.
- The **alert bit** (0x80) is set in every error byte while Hardware Error Status (70)
  is not 0.
- REBOOT and FACTORY_RESET answer first, then act. After an ID change the answer
  still comes from the old ID.

## Writes

1. Every register the write touches (directly or through Indirect Data) is checked
   with its new value. Errors: Access (read-only, EEPROM while torque is on, EEPROM via
   Secondary ID), Data Range (value, limit registers, goal while Bus Watchdog error),
   Result Fail (Torque Enable after a shutdown). Nothing is stored if one check fails.
2. In address order, the write hook may refuse; accepted registers are stored.
3. EEPROM changes are stored through `nvm_store`; a failure is reported as Result Fail.
4. Drive Mode bit 3: a goal written with torque off switches the torque on.

Reserved addresses read as 0 and ignore writes. Accesses beyond the table are Access
Errors; READ/WRITE of length 0 is a Data Length Error.

Range rules worth knowing:

| Register | Valid values |
|---|---|
| Goal Position (116) | mode 3: Min..Max Position Limit (52/48); other modes: ±1,048,575 |
| Goal Velocity (104) | ±Velocity Limit (44) |
| Goal PWM (100) | ±PWM Limit (36) |
| Goal Current (102) | ±Current Limit (38) |
| Operating Mode (11) | modes of the model |
| Drive Mode (10) | bits 0, 2, 3 |
| Indirect Address | 64 .. 661 |

## Indirect Address / Data

Two blocks of 28 entries: Indirect Address 1-28 (168..222) ↔ Indirect Data 1-28
(224..251) and 29-56 (578..632) ↔ (634..661). An Indirect Data byte behaves like the
byte it points at (size, range and access are those of the target register). A byte
that points at Indirect Data itself is plain storage.

## Lifecycle

| Situation | EEPROM (0-63) | RAM (64-661) |
|---|---|---|
| `x430_device_init` | factory values, then the stored block | factory values, then Startup Configuration |
| REBOOT | kept | factory values, then Startup Configuration; errors cleared |
| FACTORY_RESET 0xFF / 0x01 / 0x02 | factory (keep ID / keep ID and baud) | as REBOOT |
| BACKUP 0x01 | – | whole table stored, Backup Ready = 1 |
| BACKUP 0x02 (torque off) | restored except ID, Baud, Protocol | restored except Torque Enable, Bus Watchdog |

Startup Configuration (60): bit 1 restores the RAM area from the backup, bit 0 switches
the torque on (through the write hook).

## Bus Watchdog

With Torque Enable = 1 and Bus Watchdog (98) = 1..127, the device expects an
instruction packet every value × 20 ms. When the time runs out (checked in
`x430_bus_poll` and on the next packet), Bus Watchdog reads 0xFF (-1), the
`X430_EVENT_BUS_WATCHDOG` event fires and goal registers answer Data Range Error until
Bus Watchdog is written again.

## Group instructions

- **SYNC_READ / BULK_READ**: one status packet per hosted device, in request order.
- **FAST_SYNC_READ / FAST_BULK_READ**: one packet with ID 0xFE; per device
  `[error, ID, data, CRC]`, where each CRC covers the packet up to that point and the
  last one is the packet CRC. The packet is not byte-stuffed, as DynamixelSDK expects.
  IDs that are not hosted are skipped.
- **SYNC_WRITE / BULK_WRITE**: applied to the hosted IDs; a malformed BULK_WRITE is
  ignored as a whole.

## CLEAR

Option 0x01 with the magic bytes `44 58 4C 22` clears the multi-turn information.
Refused with Result Fail while Moving (122) is 1. The event hook does the work (the
simulator wraps its shaft); without an event hook the library wraps Present Position
into 0..4095.
