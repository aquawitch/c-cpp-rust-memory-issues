/* C11 version: run one deliberate bug per process under sanitizers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(int argc, char **argv) {
    if (argc != 2) return 2;
    if (strcmp(argv[1], "uaf") == 0) {
        int *p = malloc(sizeof *p);
        if (!p) return 3;
        *p = 42; free(p); printf("%d\n", *p);
    } else if (strcmp(argv[1], "double-free") == 0) {
        void *p = malloc(64); free(p); free(p);
    } else if (strcmp(argv[1], "overflow") == 0) {
        char buf[8]; strcpy(buf, "input longer than eight bytes"); puts(buf);
    } else if (strcmp(argv[1], "leak") == 0) {
        char *volatile p = malloc(1024);
        if (!p) return 3;
        p[0] = 'x'; return 0;
    } else if (strcmp(argv[1], "null") == 0) {
        int *p = NULL; printf("%d\n", *p);
    } else return 2;
    return 0;
}
