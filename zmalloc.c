#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include "zmalloc.h"


static size_t used_memory = 0;
pthread_mutex_t used_memory_mutex = PTHREAD_MUTEX_INITIALIZER;

static void zmalloc_default_oom(size_t size) {
    fprintf(stderr, "zmalloc: Out of memory trying to allocate %zu bytes\n",
        size);
    fflush(stderr);
    abort();
}

static void (*zmalloc_oom_handler)(size_t) = zmalloc_default_oom;


//package malloc
void *zmalloc(size_t size) {
    void *ptr = malloc(size);

    if (!ptr) zmalloc_oom_handler(size);
    return ptr;
}

//packege calloc
void *zcalloc(size_t size) {
    void *ptr = calloc(1, size);

    if (!ptr) zmalloc_oom_handler(size);
    return ptr;
}

//package realloc
void *zrealloc(void *ptr, size_t size) {
    void *realptr;
    realptr = realloc(ptr,size);
    if (!realptr) zmalloc_oom_handler(size);
    return realptr;
}


//package free
void zfree(void *ptr) {
    free(ptr);
}

int zmalloc_test(int argc, char **argv) {
    return 0;
}
