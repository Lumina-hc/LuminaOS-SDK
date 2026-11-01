#include <app_libc.h>

#define BUF_CAP 4095

static char text[BUF_CAP + 1];
static int text_len = 0;
static int caret = 0;

static int n_rows(int max_chars)
{
    int rows = 0, col = 0;
    for (int i = 0; i < text_len; i++) {
        char c = text[i];
        if (c == '\n') {
            rows++; col = 0;
        } else if (c != '\r') {
            col++;
            if (col >= max_chars) {
                rows++;
                col = 0;
            }
        }
    }
    if (col > 0) rows++;
    if (text_len > 0 && text[text_len - 1] == '\n') rows++;
    if (rows < 1) rows = 1;
    return rows;
}

static int n_line_start(int line, int max_chars)
{
    int ln = 0, col = 0;
    for (int i = 0; i < text_len; i++) {
        if (ln == line) return i;
        char c = text[i];
        if (c == '\n') {
            ln++; col = 0;
        } else if (c != '\r') {
            col++;
            if (col >= max_chars) {
                ln++; col = 0;
            }
        }
    }
    return text_len;
}

static void n_caret_row_col(int max_chars, int* row, int* col)
{
    int r = 0, c = 0;
    for (int i = 0; i < caret; i++) {
        char ch = text[i];
        if (ch == '\n') {
            r++; c = 0;
        } else if (ch != '\r') {
            c++;
            if (c >= max_chars) {
                r++; c = 0;
            }
        }
    }
    *row = r;
    *col = c;
}

static int n_caret_from_click(int row, int col, int max_chars)
{
    int r = 0, c = 0;
    for (int i = 0; i < text_len; i++) {
        char ch = text[i];
        if (r == row && c >= col) return i;
        if (ch == '\n') {
            r++; c = 0;
        } else if (ch != '\r') {
            c++;
            if (c >= max_chars) {
                r++; c = 0;
            }
        }
    }
    return text_len;
}

static void n_insert(char c)
{
    if (text_len >= BUF_CAP) return;
    for (int i = text_len; i > caret; i--) text[i] = text[i - 1];
    text[caret] = c;
    text_len++;
    caret++;
}

static void n_backspace(void)
{
    if (caret <= 0) return;
    for (int i = caret - 1; i < text_len - 1; i++) text[i] = text[i + 1];
    text_len--;
    caret--;
}

void _start(void)
{
    win_info_t wi;
    sys_win_info(&wi);

    char name[64];
    int has_name = sys_get_open_file(name, sizeof(name)) > 0;
    int saved_feedback = 0; /* 0 none, 1 saved, -1 failed */

    if (has_name) {
        int n = file_read(name, text, BUF_CAP);
        if (n < 0) n = 0;
        text_len = n;
        caret = n;
    } else {
        text_len = 0;
        caret = 0;
    }

    int margin = 16;
    int header_h = 36;
    int max_chars = (wi.client_w - margin - 20) / 8;
    if (max_chars < 8) max_chars = 8;
    if (max_chars > 160) max_chars = 160;

    int row_h = 16;
    int content_y = header_h + 8;
    int visible = (wi.client_h - content_y - 18) / row_h;
    if (visible < 1) visible = 1;

    int save_x = wi.client_w - 150;
    int save_y = 6;
    int save_w = 72;
    int save_h = 24;

    int scroll = 0;
    int last_over_save = -1;

    int sb_h = wi.client_h - content_y - 8;
    int sb_x = wi.client_w - 14;

    for (;;) {
        int mx = mouse_x() - wi.client_x;
        int my = mouse_y() - wi.client_y;

        int over_save_now = (mx >= save_x && mx < save_x + save_w && my >= save_y && my < save_y + save_h);
        int dirty = (over_save_now != last_over_save || saved_feedback != 0);
        last_over_save = over_save_now;

        if (sys_win_minimized()) {
            sys_minimize();
            dirty = 1;
        }

        int close = 0;
        int k;
        while ((k = getchar_nb()) != -1) {
            if (k == 27) {
                close = 1;
                break;
            }
            int changed = 0;
            if (k == '\b') {
                n_backspace();
                changed = 1;
            } else if (k == '\n') {
                n_insert('\n');
                changed = 1;
            } else if (k == '\t') {
                for (int i = 0; i < 4; i++) n_insert(' ');
                changed = 1;
            } else if (k >= 32 && k <= 126) {
                n_insert((char)k);
                changed = 1;
            }
            if (changed) {
                saved_feedback = 0;
                dirty = 1;
            }
        }
        if (close) break;

        if (dirty) {
            int rows = n_rows(max_chars);
            int max_scroll = rows - visible;
            if (max_scroll < 0) max_scroll = 0;

            int crow, ccol;
            n_caret_row_col(max_chars, &crow, &ccol);
            if (crow < scroll) scroll = crow;
            else if (crow >= scroll + visible) scroll = crow - visible + 1;
            if (scroll < 0) scroll = 0;
            if (scroll > max_scroll) scroll = max_scroll;
        }

        if (mouse_click()) {
            dirty = 1;
            saved_feedback = 0;
            int rows = n_rows(max_chars);
            int max_scroll = rows - visible;
            if (max_scroll < 0) max_scroll = 0;

            if (mx >= save_x && mx < save_x + save_w && my >= save_y && my < save_y + save_h) {
                const char* path = has_name ? name : "NOTES.TXT";
                int ok = file_write(path, text, text_len) == 0;
                saved_feedback = ok ? 1 : -1;
                if (ok && !has_name) {
                    int i = 0;
                    while (path[i] && i < (int)sizeof(name) - 1) { name[i] = path[i]; i++; }
                    name[i] = '\0';
                    has_name = 1;
                }
            } else if (rows > visible) {
                if (mx >= sb_x && mx < sb_x + 8) {
                    if (my >= content_y && my < content_y + 10) {
                        if (scroll > 0) scroll--;
                    } else if (my >= content_y + sb_h - 10 && my < content_y + sb_h) {
                        if (scroll < max_scroll) scroll++;
                    } else if (my >= content_y + 10 && my < content_y + sb_h - 10) {
                        int track = sb_h - 20;
                        int thumb = track * visible / rows;
                        if (thumb < 8) thumb = 8;
                        if (thumb > track) thumb = track;
                        if (track > thumb) {
                            int y0 = content_y + 10;
                            int t = (my - y0 - thumb / 2) * max_scroll / (track - thumb);
                            if (t < 0) t = 0;
                            if (t > max_scroll) t = max_scroll;
                            scroll = t;
                        }
                    }
                } else {
                    int row = (my - content_y) / row_h;
                    int col = (mx - margin) / 8;
                    if (row < 0) row = 0;
                    if (col < 0) col = 0;
                    if (my >= content_y && row < rows) {
                        caret = n_caret_from_click(row + scroll, col, max_chars);
                        dirty = 1;
                    }
                }
            } else {
                int row = (my - content_y) / row_h;
                int col = (mx - margin) / 8;
                if (row < 0) row = 0;
                if (col < 0) col = 0;
                if (my >= content_y && row < rows) {
                    caret = n_caret_from_click(row + scroll, col, max_chars);
                    dirty = 1;
                }
            }
        }

        if (sys_win_should_close())
            break;

        if (dirty) {
            gfx_clear(0xF7F7F7);

            int rows = n_rows(max_chars);
            int max_scroll = rows - visible;
            if (max_scroll < 0) max_scroll = 0;
            int crow, ccol;
            n_caret_row_col(max_chars, &crow, &ccol);
            if (scroll > max_scroll) scroll = max_scroll;

            gfx_fill_rect(0, 0, wi.client_w, header_h, 0x4040A0);
            gfx_draw_text(12, 10, has_name ? name : "Notepad", 0xFFFFFF);
            gfx_fill_rect(0, header_h, wi.client_w, 2, 0x202060);

            int over_save = (mx >= save_x && mx < save_x + save_w && my >= save_y && my < save_y + save_h);
            gfx_fill_rect(save_x, save_y, save_w, save_h, over_save ? 0x8090D0 : 0x5664B0);
            gfx_draw_rect(save_x, save_y, save_w, save_h, 0xC8D0F0);
            gfx_draw_text(save_x + 17, save_y + 5, "Save", 0xFFFFFF);

            if (saved_feedback == 1)
                gfx_draw_text(wi.client_w - 70, save_y + 5, "(saved)", 0xA0FFA0);
            else if (saved_feedback == -1)
                gfx_draw_text(wi.client_w - 70, save_y + 5, "(failed)", 0xFF8080);

            for (int rr = 0; rr < visible; rr++) {
                int line = scroll + rr;
                if (line >= rows) break;
                int start = n_line_start(line, max_chars);
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
                    char buf[200];
                    int b = 0;
                    for (int j = 0; j < len && b < 199; j++) {
                        char c = text[start + j];
                        if (c == '\r') continue;
                        buf[b++] = c;
                    }
                    buf[b] = '\0';
                    gfx_draw_text(margin, content_y + rr * row_h, buf, 0x202020);
                }
            }

            if (crow >= scroll && crow < scroll + visible) {
                int cy = content_y + (crow - scroll) * row_h;
                int cx = margin + ccol * 8;
                if (cx >= 0 && cx < wi.client_w - 8) {
                    gfx_fill_rect(cx, cy, 8, row_h, 0x202060);
                    if (caret < text_len)
                        gfx_draw_char(cx, cy, text[caret], 0xFFFFFF);
                }
            }

            if (rows > visible) {
                gfx_fill_rect(sb_x, content_y, 8, sb_h, 0xD8D8D8);
                gfx_fill_rect(sb_x, content_y + 4, 8, 6, 0x808080);
                gfx_fill_rect(sb_x, content_y + sb_h - 10, 8, 6, 0x808080);
                int track = sb_h - 20;
                int thumb = track * visible / rows;
                if (thumb < 8) thumb = 8;
                if (thumb > track) thumb = track;
                int pos = (max_scroll > 0) ? (scroll * (track - thumb) / max_scroll) : 0;
                gfx_fill_rect(sb_x, content_y + 10 + pos, 8, thumb, 0x4F7FBF);
            }

            gfx_flush();
        }

        sys_sleep(5);
    }

    sys_exit();
}