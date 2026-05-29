# PetEgg Jieli 701N Minimal Port

This directory is the P13 minimal port skeleton for integrating the PetEgg portable
architecture into the Jieli AC701N / BR28 watch SDK.

## Scope

- `portable/` is a vendor copy of the simulator portable C ABI, split by logical layer:
  - `system`: platform-independent system core such as result/config/types, save ABI,
    save transaction, packet, NFC pair, BT session, CRC, and resource manifest.
  - `game`: pet species, unlock table, pet model/pool, NFC content-card activation,
    care/growth, battle/Boss/QTE/reward summaries.
  - `render`: RGB565 framebuffer, draw/blit, sprite, animation, dirty rect, render perf.
  - `app_flow`: scene id, display owner, input dispatch, and action flags.
- `jieli_port/` is the only layer intended to touch Jieli LCD/key/VM/Flash/NFC/BLE/audio/power
  APIs in later stages. P13 keeps all functions as stubs or fake injection helpers.
- `app/` contains small smoke entry points and display-owner glue only.
- `tests/` contains compile-oriented smoke functions that avoid file IO, Flash IO, LCD flush,
  real NFC, real BLE, malloc, and printf.

## Product Rules

- SD card gameplay is canceled.
- Content card means a physical NFC card.
- NFC content cards only activate content already stored on local device storage or external
  Flash; they do not carry pictures, animations, pets, item resources, or config resources.
- Device-local pet capacity is `PET_MAX_COUNT = 18`.
- Pet data, card bag, equipment, home assets, and NFC activation records are local device data.
- Pets do not die, and RPG job fields such as attack, defense, speed, crit, or healing are not
  part of this port.

## Build Integration

P13 does not replace the existing watch app and does not modify the SDK root build. The proposed
next integration step is to add the source lists documented in
`SOURCE/petegg/PETEGG_JIELI_BUILD_NOTES.md` behind `CONFIG_PETEGG_MINIMAL_PORT`.

Simulator adapter code is intentionally not present in this directory.
