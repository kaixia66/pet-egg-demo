# MVP-A Resource Drop Point

This directory is the firmware-side landing point for the Google Drive package
`MVP-A_Final_Engineering_Resources_V0.1`.

The runtime mapping is centralized in `apps/watch/mvp_a/services/mvp_a_assets.c`.
Page code should request backgrounds, character stages, UI components, icons, and
animation frame groups through `mvp_a_assets` instead of hardcoding asset paths.

Current `master` status:

- PNG runtime loading is not wired in this SDK layer.
- `mvp_a_assets` returns stable metadata and fallback paths.
- A few fallback PNG files and `mvp_a_image_assets.c` C-array images are kept only
  as a bring-up bridge for the LVGL MVP-A skeleton.
- Do not expand the C-array image route for production resources.

Production resource direction:

- Convert images with the Jieli SDK resource tools / `image_dll` into hardware
  recognizable compressed formats.
- Put formal image resources in the 16M external Flash.
- Do not decode PNG or JSON at runtime.
- If IMB can read a resource directly from NOR Flash, prefer passing it via
  `flash_file_info` / address mapping.
- Avoid rotating resources directly from Flash; real-time rotation is only for
  small images or resources loaded into SRAM first.

When converted resources are added, keep the package-relative layout from Drive:

- `assets/background_scenes`
- `assets/character_p0`
- `assets/ui_components`
- `assets/motion`
- `manifests/asset_manifest.json`
- `manifests/animation_manifest.json`
- `copy_cn.json`
- `resource_index.csv`
