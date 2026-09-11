# Official LVGL source provenance

- Repository: https://github.com/lvgl/lvgl
- Release: v9.5.0 (official latest release checked 2026-09-09)
- Commit: 85aa60d18b3d5e5588d7b247abf90198f07c8a63
- Archive: https://codeload.github.com/lvgl/lvgl/tar.gz/refs/tags/v9.5.0
- Imported upstream subset: `src/`, `lvgl.h`, `lvgl_private.h`, `lv_version.h`,
  `lv_conf_template.h`, `LICENCE.txt`, `COPYRIGHTS.md` (1,136 files).
- No upstream tests, examples, demos, or Keil LVGL Pack are imported.

The upstream subset is unmodified. `lv_conf.h` is this application's explicit
configuration, not an upstream file. The manual HAL port and UI are in the
adjacent `LVGL_UI` directory. Upstream `LICENCE.txt` is MIT; incorporated library
notices listed in `COPYRIGHTS.md` and their file-local terms remain controlling.

The optional Keil LVGL target has exactly three source-local warning options:
`-Wno-unused-function` on the RGB565 and RGB565-swapped software blend files
(unused conversion helpers when other source color formats are disabled), and
`-Wno-unused-parameter` on `lv_tlsf.c` (logging-disabled diagnostic callback).
There is no target-wide warning suppression and no vendor-code patch.
Zero-warning build reports are subject to these documented options.

Read-only byte verification against the official fixed-commit archive:
`python mcu/scripts/verify_lvgl_upstream.py <lvgl-85aa60d18b3d5e5588d7b247abf90198f07c8a63.tar.gz>`.
Use the archive from `https://codeload.github.com/lvgl/lvgl/tar.gz/85aa60d18b3d5e5588d7b247abf90198f07c8a63`.
The 1,136-file comparison passed on 2026-09-11; local configuration and provenance
are deliberately excluded.
