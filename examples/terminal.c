#include <app_libc.h>

#define FONT_W 8
#define FONT_H 16
#define MAX_LINES 300
#define LINE_W 200

#define COL_BG     0x0C0E14
#define COL_TEXT   0xD8E0F0
#define COL_PROMPT 0x4CFF9F
#define COL_DIM    0x8A93A6
#define COL_ACCENT 0x4C8DFF

static char g_lines[MAX_LINES][LINE_W];
static int g_nlines = 0;
static char g_input[LINE_W];
static int g_ilen = 0;

static int s_len(const char* s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

static int s_eq(const char* a, const char* b)
{
    int i = 0;
    while (a[i] && b[i]) {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return a[i] == b[i];
}

static void add_line(const char* s)
{
    if (g_nlines >= MAX_LINES) {
        for (int i = 0; i < MAX_LINES - 1; i++)
            for (int j = 0; j < LINE_W; j++) g_lines[i][j] = g_lines[i + 1][j];
        g_nlines = MAX_LINES - 1;
    }
    int i = 0;
    while (s[i] && i < LINE_W - 1) {
        g_lines[g_nlines][i] = s[i];
        i++;
    }
    g_lines[g_nlines][i] = '\0';
    g_nlines++;
}

static void itoa_u(unsigned long v, char* buf)
{
    char tmp[24];
    int n = 0;
    if (v == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    while (v > 0 && n < 23) { tmp[n++] = (char)('0' + (v % 10)); v /= 10; }
    for (int i = 0; i < n; i++) buf[i] = tmp[n - 1 - i];
    buf[n] = '\0';
}

static void cmd_ls(void)
{
    static dir_entry_t entries[64];
    int n = file_list(entries, sizeof(entries));
    if (n <= 0) { add_line("  (filesystem unavailable or empty)"); return; }
    for (int i = 0; i < n; i++) {
        char buf[64];
        int k = 0;
        buf[k++] = ' ';
        buf[k++] = ' ';
        for (int j = 0; j < 12 && entries[i].name[j] != ' '; j++)
            buf[k++] = entries[i].name[j];
        if (entries[i].attr & 0x10) {
            const char* tag = "  [DIR]";
            for (int t = 0; tag[t]; t++) buf[k++] = tag[t];
        } else {
            const char* tag = "  ";
            for (int t = 0; tag[t]; t++) buf[k++] = tag[t];
            char num[24];
            itoa_u(entries[i].size, num);
            for (int t = 0; num[t]; t++) buf[k++] = num[t];
            const char* b = " B";
            for (int t = 0; b[t]; t++) buf[k++] = b[t];
        }
        buf[k] = '\0';
        add_line(buf);
    }
}

static void cmd_cat(const char* arg)
{
    if (!arg[0]) { add_line("usage: cat <file>"); return; }
    static char buf[2048];
    int n = file_read(arg, buf, sizeof(buf) - 1);
    if (n < 0) { add_line("  cannot open file"); return; }
    buf[n] = '\0';
    add_line(buf);
}

static void cmd_mem(void)
{
    sysinfo_t si;
    sys_sysinfo(&si);
    char num[24];
    char buf[96];

    int k = 0;
    const char* a = "  Total RAM : ";
    for (int i = 0; a[i]; i++) buf[k++] = a[i];
    itoa_u(si.mem_total_kb, num);
    for (int i = 0; num[i]; i++) buf[k++] = num[i];
    const char* u = " KB";
    for (int i = 0; u[i]; i++) buf[k++] = u[i];
    buf[k] = '\0';
    add_line(buf);

    k = 0;
    const char* b = "  Used RAM  : ";
    for (int i = 0; b[i]; i++) buf[k++] = b[i];
    itoa_u(si.mem_used_kb, num);
    for (int i = 0; num[i]; i++) buf[k++] = num[i];
    for (int i = 0; u[i]; i++) buf[k++] = u[i];
    buf[k] = '\0';
    add_line(buf);

    k = 0;
    const char* c = "  Free RAM  : ";
    for (int i = 0; c[i]; i++) buf[k++] = c[i];
    itoa_u(si.mem_free_kb, num);
    for (int i = 0; num[i]; i++) buf[k++] = num[i];
    for (int i = 0; u[i]; i++) buf[k++] = u[i];
    buf[k] = '\0';
    add_line(buf);

    k = 0;
    const char* d = "  Heap used : ";
    for (int i = 0; d[i]; i++) buf[k++] = d[i];
    itoa_u(si.heap_used_kb, num);
    for (int i = 0; num[i]; i++) buf[k++] = num[i];
    for (int i = 0; u[i]; i++) buf[k++] = u[i];
    buf[k] = '\0';
    add_line(buf);
}

static void execute(const char* raw)
{
    char cmd[LINE_W];
    int n = 0;
    while (raw[n] && n < LINE_W - 1) { cmd[n] = raw[n]; n++; }
    cmd[n] = '\0';

    int i = 0;
    while (cmd[i] == ' ' || cmd[i] == '\t') i++;
    int start = i;
    while (cmd[i] && cmd[i] != ' ') i++;
    int end = i;
    if (cmd[end]) { cmd[end] = '\0'; i = end + 1; }
    else i = end;
    while (cmd[i] == ' ') i++;
    const char* arg = cmd + i;

    const char* name = cmd + start;

    if (name[0] == '\0') return;
    if (s_eq(name, "help")) {
        add_line("  Commands:");
        add_line("    help          this list");
        add_line("    clear         clear the screen");
        add_line("    ls            list files");
        add_line("    cat <file>    show a file");
        add_line("    echo <text>   print text");
        add_line("    mem           memory usage");
        add_line("    ps            list processes");
        add_line("    pipe          pipe demo");
        add_line("    ver           OS version");
        add_line("    about         about LuminaOS");
        add_line("    exit          close terminal");
    } else if (s_eq(name, "clear")) {
        g_nlines = 0;
    } else if (s_eq(name, "ls") || s_eq(name, "dir")) {
        cmd_ls();
    } else if (s_eq(name, "cat") || s_eq(name, "type")) {
        cmd_cat(arg);
    } else if (s_eq(name, "echo")) {
        add_line(arg);
    } else if (s_eq(name, "mem") || s_eq(name, "free")) {
        cmd_mem();
    } else if (s_eq(name, "ps")) {
        int c = sys_list_procs();
        char buf[32];
        itoa_u((unsigned int)c, buf);
        char out[64];
        int k = 0;
        const char* pfx = "  procs: ";
        for (int i = 0; pfx[i]; i++) out[k++] = pfx[i];
        for (int i = 0; buf[i]; i++) out[k++] = buf[i];
        out[k] = '\0';
        add_line(out);
    } else if (s_eq(name, "pipe")) {
        int fds[2];
        if (sys_pipe(fds) < 0) {
            add_line("  pipe failed");
            return;
        }
        sys_spawn("WORKER.ELF", "worker", fds[1]);
        char r[256];
        int got = 0;
        for (int t = 0; t < 80; t++) {
            int n = sys_pipe_read(fds[0], r, 255);
            if (n > 0) {
                r[n] = '\0';
                char o[256];
                int k = 0;
                const char* pfx = "  pipe: ";
                for (int i = 0; pfx[i]; i++) o[k++] = pfx[i];
                for (int i = 0; r[i] && k < 250; i++) o[k++] = r[i];
                o[k] = '\0';
                add_line(o);
                got++;
            }
            if (got >= 3) break;
            sys_sleep(30);
        }
        sys_close(fds[0]);
        sys_close(fds[1]);
    } else if (s_eq(name, "ver")) {
        add_line("  LuminaOS v0.7.5 (x86_64)");
        add_line("  Built on the LSP SDK.");
    } else if (s_eq(name, "about")) {
        add_line("  LuminaOS - a small hobby operating system, x86_64, FAT16.");
        add_line("  GUI: Lumina Desktop.  Shell: this terminal.");
    } else if (s_eq(name, "exit") || s_eq(name, "quit")) {
        sys_exit();
    } else {
        char buf[96];
        int k = 0;
        const char* e = "  unknown command: ";
        for (int j = 0; e[j]; j++) buf[k++] = e[j];
        for (int j = 0; name[j] && k < 90; j++) buf[k++] = name[j];
        buf[k] = '\0';
        add_line(buf);
    }
}

static void render(win_info_t* wi, int cols, int rows)
{
    gfx_clear(COL_BG);

    int top = 8;
    int visible = rows - 1;
    if (visible < 1) visible = 1;
    int first = g_nlines - visible;
    if (first < 0) first = 0;

    int y = top;
    for (int i = first; i < g_nlines; i++) {
        gfx_draw_text(8, y, g_lines[i], COL_TEXT);
        y += FONT_H;
    }

    /* input line with prompt */
    int py = top + visible * FONT_H;
    const char* prompt = "Lumina> ";
    gfx_draw_text(8, py, prompt, COL_PROMPT);
    int px = 8 + s_len(prompt) * FONT_W;
    gfx_draw_text(px, py, g_input, COL_TEXT);

    int cx = px + g_ilen * FONT_W;
    if (cx < wi->client_w - 12)
        gfx_fill_rect(cx, py, 8, FONT_H - 2, COL_ACCENT);
}

void _start(void)
{
    win_info_t wi;
    sys_win_info(&wi);

    int cols = (wi.client_w - 16) / FONT_W;
    int rows = (wi.client_h - 46) / FONT_H;
    if (cols < 8) cols = 8;
    if (rows < 2) rows = 2;

    add_line("LuminaOS Terminal v0.7.5");
    add_line("Type 'help' for a list of commands.");
    add_line("");

    int dirty = 1;

    for (;;) {
        if (sys_win_minimized()) {
            sys_minimize();
            dirty = 1;
        }

        int c;
        while ((c = getchar_nb()) != -1) {
            if (c == '\n' || c == '\r') {
                char echo[LINE_W + 12];
                int k = 0;
                const char* p = "Lumina> ";
                for (int i = 0; p[i]; i++) echo[k++] = p[i];
                for (int i = 0; g_input[i] && k < LINE_W + 8; i++) echo[k++] = g_input[i];
                echo[k] = '\0';
                add_line(echo);
                execute(g_input);
                g_ilen = 0;
                g_input[0] = '\0';
                add_line("");
                dirty = 1;
            } else if (c == '\b') {
                if (g_ilen > 0) { g_ilen--; g_input[g_ilen] = '\0'; dirty = 1; }
            } else if (c >= 32 && c <= 126) {
                if (g_ilen < cols - 2 && g_ilen < LINE_W - 1) {
                    g_input[g_ilen++] = (char)c;
                    g_input[g_ilen] = '\0';
                    dirty = 1;
                }
            }
        }

        if (sys_win_should_close())
            break;

        if (dirty) {
            render(&wi, cols, rows);
            gfx_flush();
            dirty = 0;
        }

        sys_cursor();
        sys_sleep(10);
    }

    sys_exit();
}
