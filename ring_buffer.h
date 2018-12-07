#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <stdint.h>

#define DEFAULT_BUFFER_SIZE 1024

typedef struct ring_buffer ring_buffer;

struct ring_buffer {
    size_t elem_size;
    _Atomic long read_cursor;
    _Atomic long write_cursor;
    _Atomic long count;
    // mark: 0 - ready-to-write; 1 - ready-to-read
    _Atomic uint8_t buf_mark[DEFAULT_BUFFER_SIZE/8];
    void * buf;
};

extern ring_buffer * ring_buffer_new(size_t elem_size);
extern void ring_buffer_free(ring_buffer * buf);
extern int ring_buffer_read(ring_buffer * buf, void * dest);
extern int ring_buffer_write(ring_buffer * buf, void * src);

#endif
