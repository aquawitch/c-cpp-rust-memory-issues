/* C11 version: run one deliberate bug per process under sanitizers. */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Intentionally non-atomic: pthread_join does not protect concurrent writes. */
static void *increment(void *arg) {
    int *counter = arg;
    for (int i = 0; i < 100000; ++i) ++*counter;
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) return 2;
    if (strcmp(argv[1], "uaf") == 0) {
        int *p = malloc(sizeof *p);
        if (!p) return 3;
        *p = 42; free(p); printf("%d\n", *p);
    } else if (strcmp(argv[1], "double-free") == 0) {
        void *p = malloc(64); free(p); free(p);
    } else if (strcmp(argv[1], "iterator") == 0) {
        int *items = malloc(4 * sizeof *items);
        if (!items) return 3;
        for (int i = 0; i < 4; ++i) items[i] = i + 1;
        int *cursor = &items[1]; /* Pointer used to traverse the old buffer. */
        int *grown = realloc(items, 1024 * 1024 * sizeof *items);
        if (!grown) { free(items); return 3; }
        items = grown;
        /* Successful realloc invalidates cursor, even at the same address.
           ASan normally moves this allocation and reports use-after-free.
           Correct code would recreate cursor as &items[1] here. */
        printf("%d\n", *cursor);
        free(items);
    } else if (strcmp(argv[1], "race") == 0) {
        int counter = 0;
        pthread_t a, b;
        if (pthread_create(&a, NULL, increment, &counter) != 0) return 3;
        if (pthread_create(&b, NULL, increment, &counter) != 0) {
            pthread_join(a, NULL);
            return 3;
        }
        pthread_join(a, NULL);
        pthread_join(b, NULL);
        printf("%d\n", counter); /* Undefined behavior, not just lost increments. */
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
