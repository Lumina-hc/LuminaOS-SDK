#include <app_libc.h>

static void draw_centered(int y, const char* s)
{
    win_info_t wi;
    sys_win_info(&wi);
    int len = 0;
    while (s[len]) len++;
    int x = (wi.client_w - len * 8) / 2;
    if (x < 8) x = 8;
    gfx_draw_text(x, y, s, 0x202020);
}

void _start(void)
{
    win_info_t wi;
    sys_win_info(&wi);

    gfx_clear(0xF7F7F7);

    gfx_fill_rect(0, 0, wi.client_w, 60, 0x4040A0);
    gfx_draw_text(24, 20, "LuminaOS", 0xFFFFFF);
    gfx_draw_text(24 + 9 * 8, 20, "v0.7.1", 0xD0D0FF);
    gfx_fill_rect(0, 60, wi.client_w, 2, 0x202060);

    int y = 90;
    draw_centered(y, "A small 32-bit operating system"); y += 30;
    draw_centered(y, "built for learning and fun."); y += 44;
    draw_centered(y, "Architecture : i686 (32-bit)"); y += 22;
    draw_centered(y, "Kernel       : Hybrid"); y += 22;
    draw_centered(y, "Filesystem   : FAT16"); y += 22;
    draw_centered(y, "GUI          : Lumina Desktop"); y += 44;

    gfx_draw_text(24, wi.client_h - 30, "Close with Esc or the X button", 0x808080);

    gfx_flush();

    for (;;) {
        sys_cursor();
        if (sys_win_should_close())
            break;
        sys_sleep(10);
    }

    sys_exit();
}
