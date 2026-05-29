# PetEgg Portable Vendor Copy

## Source

- Source repo: `petegg-simulator`
- Source path: `shared_portable/include` and `shared_portable/src`
- Source commit: `f30e158c6fdb3ded9c8082d56bd86fd893fef433`
- Copy date: 2026-05-29
- Target path: `apps/watch/petegg/portable`

The GitHub repository was fetched with sparse/blobless checkout because a full clone and a full
archive download were too slow for the SDK-sized repository. The local `pet-egg-demo` worktree is
on upstream `master` commit `6bf886ae2b617c4c723b0c6f68a8dba6a943c746`.

## Included Modules

Portable System Core:

- `pet_result`, `pet_config`, `pet_types`
- `pet_input`, `pet_display_profile`, `pet_platform`
- `pet_crc32`
- `pet_save_format`, `pet_save_transaction`
- `pet_packet`, `pet_nfc_pair_payload`, `pet_bt_session`
- `pet_resource_manifest`

Portable Game Core:

- `pet_nfc_payload`, `pet_card_activation`
- `pet_species_table`, `pet_unlock_table`
- `pet_model`, `pet_pool`
- `pet_care`, `pet_growth`
- `pet_battle`, `pet_boss`, `pet_qte`, `pet_reward`, `pet_game_hash`

Application Flow:

- `pet_app_flow`

Portable Render Core:

- `pet_render_types`, `pet_sprite`, `pet_anim`
- `pet_framebuffer`, `pet_draw`, `pet_dirty_rect`, `pet_render_perf`

## Excluded Modules

- Simulator adapter files under `sim/portable_adapter`
- SDL, SimBroker, simulator app/window/debug overlay/snapshot export
- PC-only portable tests and C++ compatibility tests
- Jieli SDK headers or libraries inside portable modules

## Vendor Copy Rule

Portable files should stay C-compatible and platform-free. Any future refresh must record the
new source commit, included modules, excluded modules, and compatibility checks.
