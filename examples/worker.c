#include <app_libc.h>

void _start(void)
{
    int fd = sys_getarg();
    int i;
    for (i = 0; i < 20; i++) {
        int n = sys_pipe_write(fd, "hello\n", 6);
        (void)n;
        sys_sleep(50);
    }
    sys_exit();
}
