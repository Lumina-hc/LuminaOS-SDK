# LuminaOS-SDK

The LuminaOS Software Development Kit contains everything needed to write
applications for the LuminaOS desktop. Applications are written in C and
compiled into the LuminaOS **LSP** format, which is loaded and executed by the
LuminaOS kernel.

## Contents

```
include/app_libc.h   Application API: system call wrappers and data types
app.ld               Linker script for LSP executables
build_app.ps1        Build a single .c file into an .lsp application
examples/            Example applications:
                       notepad.c  Text editor
                       files.c    File browser
                       about.c    About dialog
                       system.c   System information
README.md
LICENSE              Apache License 2.0
```

## Requirements

- Windows with PowerShell
- NASM
- i686-elf cross toolchain (`i686-elf-gcc`, `i686-elf-ld`, `i686-elf-objcopy`, `i686-elf-nm`)
- A running LuminaOS system (to deploy and run your applications)

## Building an Application

```powershell
.\build_app.ps1 -Source examples\about.c -OutputDir out
```

This produces `out\about.lsp`.

## Deploying to LuminaOS

Place the generated `.lsp` file onto the LuminaOS data disk (`hdd.img`) using
any FAT16-capable tool. On boot, the file appears as a desktop icon and can be
launched by clicking it.

## The LSP Format

An LSP file is a 16-byte header followed by a raw binary image:

| Offset | Size | Field        | Description                          |
|--------|------|--------------|--------------------------------------|
| 0      | 3    | magic        | `'L' 'S' 'P'`                        |
| 3      | 1    | version      | `1`                                  |
| 4      | 4    | entry_offset | Entry point relative to base (LE)    |
| 8      | 4    | load_size    | Size of the binary image in bytes    |
| 12     | 4    | bss_size     | Zero-initialized data size           |

The image is loaded at `0x01000000` (`USER_BASE`); the entry point is
`USER_BASE + entry_offset`. Programs run in user mode and call kernel services
through `int $0x80`.

## Application API

The API is declared in `include/app_libc.h` and covers:

- Graphics: `gfx_putpixel`, `gfx_fill_rect`, `gfx_draw_rect`, `gfx_draw_char`,
  `gfx_draw_text`, `gfx_clear`, `gfx_flush`, `gfx_w`, `gfx_h`
- Keyboard: `getchar`, `getchar_nb`
- Mouse: `mouse_x`, `mouse_y`, `mouse_left`, `mouse_right`, `mouse_click`
- Files: `file_read`, `file_write`, `file_list`
- Window: `sys_win_info`, `sys_win_should_close`, `sys_open_file`,
  `sys_get_open_file`, `sys_cursor`
- System: `sys_exit`, `sys_sleep`, `sys_sysinfo`

See `include/app_libc.h` for signatures and system call numbers.

## License

Licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE).
