#include <app_libc.h>

static void draw_dir_entry(int x, int y, const dir_entry_t* e, int hover)
{
    uint32_t fg = 0x202020;
    if (e->attr & 0x10) {
        gfx_fill_rect(x, y, 40, 32, 0xE8C860);
        gfx_fill_rect(x + 2, y + 2, 36, 28, 0xFFE080);
        gfx_draw_text(x + 10, y + 10, "DIR", 0x806000);
        fg = 0x1E3A5F;
    } else {
        gfx_fill_rect(x, y, 40, 32, 0xC0C8D0);
        gfx_fill_rect(x + 2, y + 2, 36, 28, 0xF0F4F8);
        gfx_draw_text(x + 10, y + 10, "FILE", 0x606060);
    }
    if (hover)
        gfx_draw_rect(x, y, 40, 32, 0xFFFFFF);
    else
        gfx_draw_rect(x, y, 40, 32, 0x808080);

    char name_buf[13];
    int len = 0;
    while (len < 12 && e->name[len] != ' ') { name_buf[len] = e->name[len]; len++; }
    name_buf[len] = '\0';
    gfx_draw_text(x + 50, y + 4, name_buf, fg);

    if (!(e->attr & 0x10)) {
        uint32_t size = e->size;
        char buf[16];
        int n = 0;
        if (size == 0) { buf[n++] = '0'; }
        while (size > 0) {
            buf[n++] = '0' + size % 10;
            size /= 10;
        }
        int x2 = x + 50;
        int w = n * 8;
        gfx_fill_rect(x2 + 200 - w, y + 4, w, 16, 0xF0F4F8);
        for (int i = 0; i < n; i++)
            gfx_draw_char(x2 + 200 - w + i * 8, y + 4, buf[n - 1 - i], 0x808080);
    }
}

void _start(void)
{
    win_info_t wi;
    sys_win_info(&wi);

    dir_entry_t entries[64];
    int count = file_list(entries, sizeof(entries));

    int y0 = 48;
    int item_w = 460;
    int cols = (wi.client_w - 32) / item_w;
    if (cols < 1) cols = 1;
    int x0 = 16;
    int per_col = (wi.client_h - y0 - 10) / 48;
    if (per_col < 1) per_col = 1;

    int last_mx = -1;
    int last_my = -1;

    for (;;) {
        int mx = mouse_x() - wi.client_x;
        int my = mouse_y() - wi.client_y;

        int dirty = (mx != last_mx || my != last_my);
        last_mx = mx;
        last_my = my;

        if (dirty) {
            gfx_clear(0xF7F7F7);
            gfx_fill_rect(0, 0, wi.client_w, 32, 0x4040A0);
            gfx_draw_text(16, 8, "Files - Current Directory", 0xFFFFFF);
            gfx_fill_rect(0, 32, wi.client_w, 2, 0x202060);

            if (count <= 0) {
                gfx_draw_text(24, y0, "(empty or filesystem unavailable)", 0x808080);
            } else {
                for (int i = 0; i < count; i++) {
                    int col = i / per_col;
                    int row = i % per_col;
                    int x = x0 + col * item_w;
                    int y = y0 + row * 48;
                    int hover = (mx >= x && mx < x + 40 && my >= y && my < y + 32);
                    draw_dir_entry(x, y, &entries[i], hover);
                }
            }

            gfx_flush();
        }

        if (mouse_click()) {
            for (int i = 0; i < count; i++) {
                if (entries[i].attr & 0x10) continue;
                int col = i / per_col;
                int row = i % per_col;
                int x = x0 + col * item_w;
                int y = y0 + row * 48;
                if (mx >= x && mx < x + item_w && my >= y && my < y + 32) {
                    char nm[13];
                    int n = 0;
                    while (n < 12 && entries[i].name[n] != ' ') { nm[n] = entries[i].name[n]; n++; }
                    nm[n] = '\0';
                    if (n >= 4 && nm[n - 4] == '.' &&
                        (nm[n - 3] == 't' || nm[n - 3] == 'T') &&
                        (nm[n - 2] == 'x' || nm[n - 2] == 'X') &&
                        (nm[n - 1] == 't' || nm[n - 1] == 'T')) {
                        sys_open_file(nm);
                        sys_exit();
                    }
                    break;
                }
            }
        }

        sys_cursor();
        if (sys_win_should_close())
            break;
        sys_sleep(10);
    }

    sys_exit();
}
