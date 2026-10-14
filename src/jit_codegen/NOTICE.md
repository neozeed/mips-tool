# Native instruction emitter provenance

`x86-codegen.h` and `amd64-codegen.h` were taken from the user-supplied
`dynamips-0.2.16_p2_merged2.zip` archive. Its SHA-256 is:

```text
636c88acc222c7dd2914ad64b3ed9b8a3bc4087f65897c298ae2fd3ba5110279
```

The headers identify their original authors as Paolo Molaro, Intel Corporation
(ORP Project), Sergey Chaban, Dietmar Maurer, Patrik Torstensson, and, for the
AMD64 header, Zalman Stern. Their original Intel and Ximian copyright notices
are preserved in each file. Dynamips and its MIPS translators were developed
by Christophe Fillot and contributors.

These are the Mono native-code emitter macros. Mono's historical license
document explicitly lists `mono/arch/*/XXX-codegen.h` under MIT X11. This is
separate from the license of Dynamips' own emulator and translator code.
No Dynamips MIPS translator, CPU, MMU, memory subsystem, or runtime source is
included here.

The accompanying license files were retrieved from the original Mono project
on 2026-10-04:

- `LICENSE.MONO-2.10`: https://raw.githubusercontent.com/mono/mono/mono-2-10/LICENSE
- `LICENSE.MIT-X11`: https://raw.githubusercontent.com/mono/mono/mono-2-10/mcs/MIT.X11

Local changes in R5U17:

- Normalize CRLF line endings to LF.
- Replace the three inline helper signatures' nonstandard `u_char` type with
  `unsigned char`, avoiding a dependency on a BSD typedef.
- Include `<stdint.h>` in the AMD64 header.
- Use `uint64_t` for the AMD64 immediate buffer and full-width immediate
  casts, replacing `long`, which is only 32 bits on Windows AMD64.
- Use fixed-width integer casts in immediate-width selection and replace the
  external `glong` dependency with `int64_t` in `amd64_is_imm32`.

The other macros remain as supplied. R3000 code uses a small, explicit-width
subset. This is not a claim that every inherited macro has been validated.
