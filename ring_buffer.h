#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

typedef struct ring_buffer ring_buffer;

struct ring_buffer {
    size_t elem_size;
    size_t capacity;
    _Atomic long read_cursor;
    _Atomic long write_cursor;
    _Atomic long count;
    // mark: 0 - ready-to-write; 1 - ready-to-read
    _Atomic int *buf_mark;
    void * buf;
};

extern ring_buffer * ring_buffer_new(size_t elem_size, size_t capacity);
extern void ring_buffer_free(ring_buffer * buf);
extern int ring_buffer_read(ring_buffer * buf, void * dest);
extern int ring_buffer_write(ring_buffer * buf, void * src);

#endif
