# PetEgg Jieli Build Notes

## Existing SDK Build

The root README and AGENTS documents identify the current AC701N / BR28 build flow:

```powershell
.\.vscode\winmk.bat all
make
make VERBOSE=1
make clean
```

The expected final SDK artifacts include `cpu/br28/tools/sdk.elf`, `app.bin`, `update.ufw`, and
`upgrade.zip`; these must not be committed.

## P13 Integration Status

P13 intentionally does not edit the root `Makefile`, board config, `apps/watch/app_main.c`, or
precompiled libraries. The PetEgg sources are staged as an isolated skeleton under
`apps/watch/petegg`.

Actual Jieli toolchain build was not executed on this macOS host because the SDK expects the
Jieli PI32 toolchain and Windows/Linux SDK utilities documented by the project. A host-side C99
syntax smoke is used to verify that the portable copy and stubs are self-contained.

## Proposed Source Groups

When the vendor or board owner enables the port, add these groups behind a disabled-by-default
macro such as `CONFIG_PETEGG_MINIMAL_PORT`:

```text
apps/watch/petegg/portable/system/src/*.c
apps/watch/petegg/portable/game/src/*.c
apps/watch/petegg/portable/render/src/*.c
apps/watch/petegg/portable/app_flow/src/*.c
apps/watch/petegg/jieli_port/*.c
apps/watch/petegg/app/*.c
apps/watch/petegg/tests/petegg_portable_compile_smoke.c
apps/watch/petegg/tests/petegg_jieli_port_smoke.c
```

Recommended include paths:

```text
apps/watch/petegg/portable/system/include
apps/watch/petegg/portable/game/include
apps/watch/petegg/portable/render/include
apps/watch/petegg/portable/app_flow/include
apps/watch/petegg/jieli_port
```

## Host Smoke Command

The local host syntax check used during P13 is:

```sh
cc -std=c99 -fsyntax-only \
  -Iapps/watch/petegg/portable/system/include \
  -Iapps/watch/petegg/portable/game/include \
  -Iapps/watch/petegg/portable/render/include \
  -Iapps/watch/petegg/portable/app_flow/include \
  -Iapps/watch/petegg/jieli_port \
  $(find apps/watch/petegg/portable apps/watch/petegg/jieli_port apps/watch/petegg/app apps/watch/petegg/tests -name "*.c" | sort)
```

This check does not replace board compilation or hardware validation.
