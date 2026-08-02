#include <app_libc.h>

static int n_strlen(const char* s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

static int n_line_start(const char* text, int len, int max_chars, int line)
{
    int ln = 0;
    int col = 0;
    for (int i = 0; i < len; i++) {
        if (ln == line) return i;
        char c = text[i];
        if (c == '\n') {
            ln++;
            col = 0;
        } else if (c != '\r') {
            col++;
            if (col >= max_chars) {
                ln++;
                col = 0;
            }
        }
    }
    return len;
}

static int n_line_count(const char* text, int len, int max_chars)
{
    int ln = 0;
    int col = 0;
    for (int i = 0; i < len; i++) {
        char c = text[i];
        if (c == '\n') {
            ln++;
            col = 0;
        } else if (c != '\r') {
            col++;
            if (col >= max_chars) {
                ln++;
                col = 0;
            }
        }
    }
    if (col > 0) ln++;
    return ln;
}

void _start(void)
{
    win_info_t wi;
    sys_win_info(&wi);

    char name[64];
    int has_name = sys_get_open_file(name, sizeof(name)) > 0;

    char text[4096];
    int text_len = 0;
    if (has_name) {
        text_len = file_read(name, text, sizeof(text) - 1);
        if (text_len < 0) text_len = 0;
        text[text_len] = '\0';
    }

    int margin = 16;
    int header_h = 36;
    int max_chars = (wi.client_w - margin - 20) / 8;
    if (max_chars < 8) max_chars = 8;

    int row_h = 16;
    int content_y = header_h + 8;
    int visible = (wi.client_h - content_y - 8) / row_h;
    if (visible < 1) visible = 1;

    int line_count = n_line_count(text, text_len, max_chars);
    int max_scroll = line_count - visible;
    if (max_scroll < 0) max_scroll = 0;

    int scroll = 0;
    int last_mx = -1;
    int last_my = -1;
    int last_scroll = -1;

    int sb_x = wi.client_w - 14;
    int sb_top = content_y;
    int sb_h = wi.client_h - content_y - 8;

    for (;;) {
        int mx = mouse_x() - wi.client_x;
        int my = mouse_y() - wi.client_y;

        int dirty = (mx != last_mx || my != last_my || scroll != last_scroll);
        last_mx = mx;
        last_my = my;
        last_scroll = scroll;

        if (dirty) {
            gfx_clear(0xF7F7F7);

            gfx_fill_rect(0, 0, wi.client_w, header_h, 0x4040A0);
            gfx_draw_text(12, 10, has_name ? name : "Notepad", 0xFFFFFF);
            if (has_name)
                gfx_draw_text(wi.client_w - 150, 10, "(Esc/X closes)", 0xD0D0FF);
            gfx_fill_rect(0, header_h, wi.client_w, 2, 0x202060);

            if (!has_name || text_len <= 0) {
                gfx_draw_text(24, content_y,
                              has_name ? "(file is empty)" : "(no file loaded)", 0x808080);
            } else {
                for (int row = 0; row < visible; row++) {
                    int line = scroll + row;
                    if (line >= line_count) break;
                    int start = n_line_start(text, text_len, max_chars, line);
                    int col = 0;
                    int i = start;
                    while (i < text_len && col < max_chars) {
                        char c = text[i];
                        if (c == '\n') break;
                        if (c != '\r') col++;
                        i++;
                    }
                    int len = i - start;
                    if (len > 0) {
                        char buf[160];
                        int b = 0;
                        for (int j = 0; j < len && b < 159; j++) {
                            char c = text[start + j];
                            if (c == '\r') continue;
                            buf[b++] = c;
                        }
                        buf[b] = '\0';
                        gfx_draw_text(margin, content_y + row * row_h, buf, 0x202020);
                    }
                }
            }

            int sb_x2 = wi.client_w - 14;
            int sb_top2 = content_y;
            int sb_h2 = wi.client_h - content_y - 8;
            if (line_count > visible) {
                gfx_fill_rect(sb_x2, sb_top2, 8, sb_h2, 0xD8D8D8);
                gfx_fill_rect(sb_x2, sb_top2 + 4, 8, 6, 0x808080);
                gfx_fill_rect(sb_x2, sb_top2 + sb_h2 - 10, 8, 6, 0x808080);
                int track = sb_h2 - 20;
                int thumb_h = track * visible / line_count;
                if (thumb_h < 8) thumb_h = 8;
                if (thumb_h > track) thumb_h = track;
                int pos = (max_scroll > 0) ? (scroll * (track - thumb_h) / max_scroll) : 0;
                gfx_fill_rect(sb_x2, sb_top2 + 10 + pos, 8, thumb_h, 0x4F7FBF);
            }

            gfx_flush();
        }

        sys_cursor();

        if (mouse_click()) {
            if (line_count > visible && mx >= sb_x && mx < sb_x + 8) {
                if (my >= sb_top && my < sb_top + 10) {
                    if (scroll > 0) scroll--;
                } else if (my >= sb_top + sb_h - 10 && my < sb_top + sb_h) {
                    if (scroll < max_scroll) scroll++;
                } else if (my >= sb_top + 10 && my < sb_top + sb_h - 10) {
                    int track = sb_h - 20;
                    int thumb_h = track * visible / line_count;
                    if (thumb_h < 8) thumb_h = 8;
                    if (thumb_h > track) thumb_h = track;
                    if (track > thumb_h) {
                        int y0 = sb_top + 10;
                        int t = (my - y0 - thumb_h / 2) * max_scroll / (track - thumb_h);
                        if (t < 0) t = 0;
                        if (t > max_scroll) t = max_scroll;
                        scroll = t;
                    }
                }
            }
        }

        if (sys_win_should_close())
            break;
        sys_sleep(10);
    }

    sys_exit();
}
