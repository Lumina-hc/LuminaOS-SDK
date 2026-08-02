#include <app_libc.h>

static void itoa_kb(uint32_t kb, char* buf)
{
    char tmp[16];
    int n = 0;
    if (kb == 0) { buf[0] = '0'; buf[1] = 0; return; }
    while (kb > 0) { tmp[n++] = '0' + kb % 10; kb /= 10; }
    for (int i = 0; i < n; i++) buf[i] = tmp[n - 1 - i];
    buf[n] = 0;
}

static void draw_row(int y, const char* label, uint32_t free_kb, uint32_t used_kb, uint32_t total_kb, int w)
{
    char buf[16];

    gfx_draw_text(24, y, label, 0x202020);

    int bar_x = 190;
    int bar_w = w;
    int used_w = 0;
    if (total_kb > 0) used_w = (int)((used_kb * (bar_w - 30)) / total_kb);
    gfx_fill_rect(bar_x, y + 1, bar_w - 20, 14, 0xE0E0E0);
    if (used_w > 0)
        gfx_fill_rect(bar_x, y + 1, used_w, 14, 0x4060C0);
    gfx_draw_rect(bar_x, y + 1, bar_w - 20, 14, 0x808080);

    itoa_kb(free_kb, buf);
    int fl = 0; while (buf[fl]) fl++;
    gfx_draw_text(bar_x + (bar_w - 20) - fl * 8 - 6, y + 2, buf, 0xFFFFFF);

    gfx_draw_text(bar_x + (bar_w - 20) + 12, y + 2, "free KB", 0x808080);
}

void _start(void)
{
    win_info_t wi;
    sys_win_info(&wi);
    int w = wi.client_w;
    int h = wi.client_h;

    gfx_clear(0xF7F7F7);

    gfx_fill_rect(0, 0, w, 32, 0x4040A0);
    gfx_draw_text(16, 8, "System Monitor", 0xFFFFFF);
    gfx_fill_rect(0, 32, w, 2, 0x202060);

    int y = 60;
    sysinfo_t si;
    sys_sysinfo(&si);

    draw_row(y, "Memory", si.mem_free_kb, si.mem_used_kb, si.mem_total_kb, w); y += 34;
    draw_row(y, "Heap", si.heap_free_kb, si.heap_used_kb, si.heap_free_kb + si.heap_used_kb, w); y += 40;

    gfx_draw_text(24, y, "Totals", 0x606060); y += 22;
    gfx_draw_text(40, y, "Total RAM", 0x202020);
    {
        char buf[16];
        itoa_kb(si.mem_total_kb, buf);
        gfx_draw_text(220, y, buf, 0x202020);
        gfx_draw_text(220 + 8 * 8, y, "KB", 0x808080);
    }
    y += 22;
    gfx_draw_text(40, y, "Used RAM", 0x202020);
    {
        char buf[16];
        itoa_kb(si.mem_used_kb, buf);
        gfx_draw_text(220, y, buf, 0x202020);
        gfx_draw_text(220 + 8 * 8, y, "KB", 0x808080);
    }
    y += 22;
    gfx_draw_text(40, y, "Kernel heap used", 0x202020);
    {
        char buf[16];
        itoa_kb(si.heap_used_kb, buf);
        gfx_draw_text(220, y, buf, 0x202020);
        gfx_draw_text(220 + 8 * 8, y, "KB", 0x808080);
    }

    gfx_draw_text(24, h - 30, "Close with Esc or the X button", 0x808080);

    gfx_flush();

    for (;;) {
        sys_cursor();
        if (sys_win_should_close())
            break;
        sys_sleep(10);
    }

    sys_exit();
}
