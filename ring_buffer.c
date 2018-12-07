#include "ring_buffer.h"

ring_buffer * ring_buffer_new(size_t elem_size) {
    ring_buffer * buf = malloc(sizeof(ring_buffer));
    buf->elem_size = elem_size;
    atomic_init(&buf->read_cursor, 0);
    atomic_init(&buf->write_cursor, 0);
    atomic_init(&buf->count, 0);
    for (int i = 0; i < DEFAULT_BUFFER_SIZE / 8; i++) {
        atomic_init(buf->buf_mark + i, 0);
    }
    buf = malloc(DEFAULT_BUFFER_SIZE * elem_size);
    return buf;
}

void ring_buffer_free(ring_buffer * buf) {
    free(buf->buf);
    free(buf);
}

int ring_buffer_read(ring_buffer * buf, void * dest) {
    long count = atomic_load(&buf->count);
    while (count > 0) {
        atomic_compare_exchange_strong(&buf->count, &count, count - 1);
    }
    if (count == 0) {
        return 0;
    }
    long idx = atomic_fetch_add(&buf->read_cursor, 1) % DEFAULT_BUFFER_SIZE;
    _Atomic uint8_t * mark = buf->buf_mark + idx / 8;
    // loop until ready to read
    while (!(atomic_load(mark) & (1 << idx % 8)));
    memcpy(dest, buf->buf + idx * buf->elem_size, buf->elem_size);
    // change state to ready-to-write
    atomic_fetch_and(mark, ~(1 << idx % 8));
    return 1;
}

int ring_buffer_write(ring_buffer * buf, void * src) {
    long count = atomic_load(&buf->count);
    while (count < DEFAULT_BUFFER_SIZE) {
        atomic_compare_exchange_strong(&buf->count, &count, count + 1);
    }
    if (count == DEFAULT_BUFFER_SIZE) {
        return 0;
    }
    long idx = atomic_fetch_add(&buf->write_cursor, 1) % DEFAULT_BUFFER_SIZE;
    _Atomic uint8_t * mark = buf->buf_mark + idx / 8;
    // loop until ready to write
    while (atomic_load(mark) & (1 << idx % 8));
    memcpy(buf->buf + idx * buf->elem_size, src, buf->elem_size);
    // change state to ready-to-read
    atomic_fetch_or(mark, 1 << idx % 8);
    return 1;
}
