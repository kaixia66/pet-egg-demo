# MVP-A Resource Drop Point

This directory is the firmware-side landing point for the Google Drive package
`MVP-A_Final_Engineering_Resources_V0.1`.

The runtime mapping is centralized in `apps/watch/mvp_a/services/mvp_a_assets.c`.
Page code should request backgrounds, character stages, UI components, icons, and
animation frame groups through `mvp_a_assets` instead of hardcoding asset paths.

PNG loading is not wired in this SDK layer yet, so the asset service currently
returns stable metadata and fallback paths. When converted resources are added,
keep the package-relative layout from Drive:

- `assets/background_scenes`
- `assets/character_p0`
- `assets/ui_components`
- `assets/motion`
- `manifests/asset_manifest.json`
- `manifests/animation_manifest.json`
- `copy_cn.json`
- `resource_index.csv`
