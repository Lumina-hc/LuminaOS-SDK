# LuminaOS-SDK

The LuminaOS Software Development Kit contains everything needed to write
applications for the LuminaOS desktop. Applications are written in C and
compiled to flat **ELF64** executables that the LuminaOS kernel loads into
ring 3 at `0x08048000`.

## Contents

```
include/app_libc.h   Application API: system call wrappers and data types
elf.ld               Linker script for ELF64 applications
build_app.ps1        Build a single .c file into an .elf application
examples/            Example applications:
                       notepad.c   Text editor
                       files.c     File browser
                       about.c     About dialog
                       system.c    System information
                       terminal.c  Terminal with a small command shell
                       worker.c    Pipes worker process (used by "pipe")
README.md
LICENSE              Apache License 2.0
```

## Requirements

- Windows with PowerShell
- `x86_64-elf` cross toolchain (`x86_64-elf-gcc`, `x86_64-elf-ld`,
  `x86_64-elf-objcopy`, `x86_64-elf-nm`)
- A running LuminaOS system (to deploy and run your applications)

## Building an Application

```powershell
.\build_app.ps1 -Source examples\about.c -OutputDir out
```

This produces `out\about.elf`. The main LuminaOS `build.ps1` builds every
example at once and writes the results into the FAT16 data disk.

## Deploying to LuminaOS

Copy the generated `.elf` file onto the LuminaOS data disk (`hdd.img`) with any
FAT16-capable tool, keeping the 8.3 upper-case name (for example
`ABOUT.ELF`). Applications on the data disk are launched from the desktop or
from the terminal.

## Application Format

Applications are ordinary ELF64 executables linked at `0x08048000`, with
`_start` as the entry point. The kernel maps the segments with user
permissions, allocates a 16-page stack below `0x7FF00000`, and starts the
process in ring 3. Programs call kernel services through `int $0x80`.

## Application API

The API is declared in `include/app_libc.h` and covers:

- Graphics: `gfx_putpixel`, `gfx_fill_rect`, `gfx_draw_rect`, `gfx_draw_char`,
  `gfx_draw_text`, `gfx_clear`, `gfx_flush`, `gfx_w`, `gfx_h`
- Keyboard: `getchar`, `getchar_nb`
- Mouse: `mouse_x`, `mouse_y`, `mouse_left`, `mouse_right`, `mouse_click`
- Files: `file_read`, `file_write`, `file_list`
- Window: `sys_win_info`, `sys_win_should_close`, `sys_win_minimized`,
  `sys_minimize`, `sys_open_file`, `sys_get_open_file`, `sys_cursor`
- Processes: `sys_spawn`, `sys_getarg`, `sys_getpid`, `sys_yield`, `sys_wait`,
  `sys_list_procs`
- Pipes: `sys_pipe`, `sys_pipe_read`, `sys_pipe_write`, `sys_close`
- System: `sys_exit`, `sys_sleep`, `sys_sysinfo`

Each window handles its own client area; `sys_win_info` reports the geometry of
the tile the kernel assigned to the process, so applications never hard-code
screen coordinates.

See `include/app_libc.h` for signatures and system call numbers.

## License

Licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE).
