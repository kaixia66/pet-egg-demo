# P5 Resource Manifest Adapter

## SDK Resource Path Audit

- MVP-A already has `apps/watch/mvp_a/services/mvp_a_assets.c` as a metadata/path mapping layer.
- `apps/watch/mvp_a/services/mvp_a_image_assets.c` contains temporary LVGL C-array fallback images.
  It is not the production resource route.
- The MVP-A resource drop directory is
  `apps/watch/mvp_a/resources/MVP-A_Final_Engineering_Resources_V0.1/`; its README states that PNG
  runtime loading is not wired and production resources should use Jieli resource tools / `image_dll`.
- The traditional SDK UI resource path uses `resfile.h`, `res_fopen`, `open_resfile`,
  `open_image_by_id`, `ui_res_flash_info_get`, `flash_file_info`, `JL.res`, `watch.res`, `.sty`, `.str`
  and `.view` files.
- Candidate future IMB/resource references are in `cpu/br28/ui_driver/imb_demo.c`,
  `cpu/br28/ui_driver/jpeg_demo.c`, and `cpu/br28/ui_driver/interface/ui_platform.c`.
- Resource packaging scripts live under `cpu/br28/tools/download/watch/*.bat`, and UI resource projects
  live under `cpu/br28/tools/UI工程/ui_454x454_watch`.
- Active board config enables NOR/SFC-related macros, but P5 does not read or write NOR/SFC/Flash.

## Manifest ABI Alignment

P5 follows the P1 header and simulator `pet_resource_manifest.h` ABI:
- manifest magic `0x5054524D`;
- format version `1`;
- manifest header size `16` bytes;
- serialized entry size `28` bytes;
- entries contain `resource_id`, `resource_type`, `format`, `offset`, `size`, `crc32`, `width`,
  `height`, `frame_count`, `flags` and `reserved`;
- table CRC32 is calculated over the serialized entry table.

The P1/simulator ABI has no BACKGROUND type. Resource ID 1001 is therefore encoded as an RGB565 SPRITE
with a background role noted only in this P5 fixture/report.

## Test Blob

`pet_resource_jieli_test_blob.c` contains a 272-byte fixture:
- ID 1001: 8x8 RGB565 background-role placeholder, type SPRITE, format RGB565, data size 128 bytes,
  CRC32 `0xDB158F3F`;
- ID 2001: 4x4 RGB565 sprite placeholder, type SPRITE, format RGB565, data size 32 bytes,
  CRC32 `0x6232A157`;
- ID 3001: 12-byte animation-table placeholder, type ANIMATION, format ANIMATION_TABLE,
  CRC32 `0x474BB302`;
- table CRC32 `0xBC84280E`.

This fixture is not final art and is not a production resource package.

## Parser API

`apps/watch/pet_resource_jieli/` exposes:
- `pet_resource_jieli_open_blob()`;
- `pet_resource_jieli_validate_manifest()`;
- `pet_resource_jieli_get_manifest_info()`;
- `pet_resource_jieli_find_entry_by_id()`;
- `pet_resource_jieli_find_entry_by_type_index()`;
- `pet_resource_jieli_read_entry()`;
- `pet_resource_jieli_crc32()`;
- `pet_resource_jieli_self_test()`.

The parser is C99, uses no malloc, no file IO, no Flash IO, no Jieli private headers and no image
decoder.

## Self-Test

`pet_resource_jieli_self_test()` checks invalid open parameters, valid fixture open, manifest
validation, header/entry sizes, entry count, ID lookup for all three resources, unknown ID behavior,
type/index lookup, read-entry pointer/size bounds, corrupted magic handling and corrupted data CRC
handling.

`pet2d_boundary_resource_probe_self_test()` calls the resource self-test only. It does not enter Pet2D
runtime, load sprites, allocate a framebuffer or call LCD flush.

## Boundaries

P5 does not read external Flash, write VM/Flash, decode PNG/JPG/GIF/JSON, import production art, modify
LVGL flush, change MVP-A default pages, connect BLE/NFC, or enable Pet2D runtime.

## Follow-Up

Future resource phases should define the production manifest generator, map manifest/resource binaries
to Jieli resource-tool output, confirm external-Flash address ownership and CRC/update policy, and then
validate IMB-readable resource byte order, compression, alignment and performance on hardware.
