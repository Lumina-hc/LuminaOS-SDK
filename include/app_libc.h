#ifndef APP_LIBC_H
#define APP_LIBC_H

#include <stdint.h>

#define SYSCALL_EXIT       0
#define SYSCALL_PUTPIXEL   1
#define SYSCALL_FILL_RECT  2
#define SYSCALL_DRAW_RECT  3
#define SYSCALL_DRAW_CHAR  4
#define SYSCALL_DRAW_TEXT  5
#define SYSCALL_GFX_CLEAR  6
#define SYSCALL_GFX_FLUSH  7
#define SYSCALL_GFX_W      8
#define SYSCALL_GFX_H      9
#define SYSCALL_GETCHAR   10
#define SYSCALL_GETCHAR_NB 11
#define SYSCALL_MOUSE_X   12
#define SYSCALL_MOUSE_Y   13
#define SYSCALL_MOUSE_L   14
#define SYSCALL_MOUSE_R   15
#define SYSCALL_MOUSE_CLICK 16
#define SYSCALL_FILE_READ  17
#define SYSCALL_FILE_WRITE 18
#define SYSCALL_FILE_LIST  19
#define SYSCALL_SLEEP      20
#define SYSCALL_CURSOR     21
#define SYSCALL_WIN_INFO   22
#define SYSCALL_SYSINFO    23
#define SYSCALL_WIN_SHOULD_CLOSE 24
#define SYSCALL_OPEN_FILE  25
#define SYSCALL_GET_OPEN_FILE 26

typedef struct {
    int win_x, win_y, win_w, win_h;
    int client_x, client_y, client_w, client_h;
    int close_x, close_y, close_w, close_h;
} win_info_t;

typedef struct {
    uint32_t mem_total_kb;
    uint32_t mem_free_kb;
    uint32_t mem_used_kb;
    uint32_t heap_free_kb;
    uint32_t heap_used_kb;
} sysinfo_t;

typedef struct {
    char name[12];
    uint8_t attr;
    uint8_t pad;
    uint32_t size;
} __attribute__((packed)) dir_entry_t;

static inline int syscall5(int n, int a1, int a2, int a3, int a4)
{
    int r;
    __asm__ volatile (
        "int $0x80"
        : "=a"(r)
        : "a"(n), "b"(a1), "c"(a2), "d"(a3), "S"(a4)
        : "memory");
    return r;
}

static inline void sys_exit(void) { syscall5(SYSCALL_EXIT, 0, 0, 0, 0); }
static inline void gfx_putpixel(int x, int y, uint32_t c) { syscall5(SYSCALL_PUTPIXEL, x, y, (int)c, 0); }
static inline void gfx_fill_rect(int x, int y, int w, int h, uint32_t c) { syscall5(SYSCALL_FILL_RECT, x, y, w | (h << 16), (int)c); }
static inline void gfx_draw_rect(int x, int y, int w, int h, uint32_t c) { syscall5(SYSCALL_DRAW_RECT, x, y, w | (h << 16), (int)c); }
static inline void gfx_draw_char(int x, int y, char c, uint32_t color) { syscall5(SYSCALL_DRAW_CHAR, x, y, (int)color, c); }
static inline void gfx_draw_text(int x, int y, const char* s, uint32_t color) { syscall5(SYSCALL_DRAW_TEXT, x, y, (int)s, (int)color); }
static inline void gfx_clear(uint32_t c) { syscall5(SYSCALL_GFX_CLEAR, (int)c, 0, 0, 0); }
static inline void gfx_flush(void) { syscall5(SYSCALL_GFX_FLUSH, 0, 0, 0, 0); }
static inline int gfx_w(void) { return syscall5(SYSCALL_GFX_W, 0, 0, 0, 0); }
static inline int gfx_h(void) { return syscall5(SYSCALL_GFX_H, 0, 0, 0, 0); }
static inline int getchar(void) { return syscall5(SYSCALL_GETCHAR, 0, 0, 0, 0); }
static inline int getchar_nb(void) { return syscall5(SYSCALL_GETCHAR_NB, 0, 0, 0, 0); }
static inline int mouse_x(void) { return syscall5(SYSCALL_MOUSE_X, 0, 0, 0, 0); }
static inline int mouse_y(void) { return syscall5(SYSCALL_MOUSE_Y, 0, 0, 0, 0); }
static inline int mouse_left(void) { return syscall5(SYSCALL_MOUSE_L, 0, 0, 0, 0); }
static inline int mouse_right(void) { return syscall5(SYSCALL_MOUSE_R, 0, 0, 0, 0); }
static inline int mouse_click(void) { return syscall5(SYSCALL_MOUSE_CLICK, 0, 0, 0, 0); }
static inline int file_read(const char* p, void* b, int n) { return syscall5(SYSCALL_FILE_READ, (int)p, (int)b, n, 0); }
static inline int file_write(const char* p, const void* b, int n) { return syscall5(SYSCALL_FILE_WRITE, (int)p, (int)b, n, 0); }
static inline int file_list(void* buf, int n) { return syscall5(SYSCALL_FILE_LIST, (int)buf, 0, n, 0); }
static inline void sys_sleep(int ms) { syscall5(SYSCALL_SLEEP, ms, 0, 0, 0); }
static inline void sys_cursor(void) { syscall5(SYSCALL_CURSOR, 0, 0, 0, 0); }
static inline void sys_win_info(win_info_t* info) { syscall5(SYSCALL_WIN_INFO, (int)info, 0, 0, 0); }
static inline void sys_sysinfo(sysinfo_t* info) { syscall5(SYSCALL_SYSINFO, (int)info, 0, 0, 0); }
static inline int sys_win_should_close(void) { return syscall5(SYSCALL_WIN_SHOULD_CLOSE, 0, 0, 0, 0); }
static inline void sys_open_file(const char* p) { syscall5(SYSCALL_OPEN_FILE, (int)p, 0, 0, 0); }
static inline int sys_get_open_file(char* p, int max) { return syscall5(SYSCALL_GET_OPEN_FILE, (int)p, 0, max, 0); }

#endif
