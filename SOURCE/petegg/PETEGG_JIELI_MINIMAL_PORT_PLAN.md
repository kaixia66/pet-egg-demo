# P13 Jieli 701N Minimal Port Plan

## Goal

P13 establishes a minimal PetEgg directory and Jieli port skeleton inside the AC701N / BR28 SDK.
It is not a full product integration stage.

## Architecture Boundary

- Portable modules do not include Jieli SDK, LVGL, SDL, simulator adapter, SimBroker, file IO, or
  Flash IO.
- `jieli_port` is the only layer that should later map portable contracts to Jieli LCD, key scan,
  VM/Flash, external Flash resource read, NFC reader, BLE GATT, audio, battery, sleep, and wakeup.
- The existing watch app remains authoritative. P13 does not replace `apps/watch/app_main.c`.

## P13 Stub Coverage

- Input: fake key injection maps to `PetKeyEvent`.
- Display: RGB565 rect flush stub validates arguments but does not touch LCD.
- Storage: in-memory A/B slot stub validates portable slot bytes and avoids real VM/Flash.
- Resource: resource read stub is present but unsupported until external Flash mapping is defined.
- NFC: fake content-card injection and poll stub are present; real secure tag validation is future work.
- BLE: send stub accepts packet bytes but does not use real GATT.
- Audio: event stub is present.
- Power: fake battery state injection is present.

## Product Rules

- SD card gameplay and SD world manifest are not part of this port.
- Content card means NFC physical card.
- NFC content cards only activate local or external-Flash preloaded content.
- NFC payloads carry small IDs and validation data, not resource bytes.
- `PET_MAX_COUNT = 18` remains the device-local pet capacity.
- Pets do not die.
- RPG job fields such as attack, defense, speed, crit, and healing are not introduced.

## Future Route

- P14 Jieli Input + Display POC
- P15 Jieli Storage + Save Slot
- P16 Jieli Resource Manifest / External Flash
- P17 Jieli Fake NFC -> Real NFC
- P18 Jieli BLE Packet Bridge
- P19 Jieli Pet2D Home POC
- P20 Two-board Boss POC
