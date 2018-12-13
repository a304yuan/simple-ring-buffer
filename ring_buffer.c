#include "ring_buffer.h"

ring_buffer * ring_buffer_new(size_t elem_size, size_t capacity) {
    ring_buffer * buf = malloc(sizeof(ring_buffer));
    buf->elem_size = elem_size;
    buf->capacity = capacity;
    atomic_init(&buf->read_cursor, 0);
    atomic_init(&buf->write_cursor, 0);
    atomic_init(&buf->count, 0);
    buf->buf = malloc(capacity * elem_size);
    buf->buf_mark = malloc(capacity * sizeof(int));
    for (int i = 0; i < capacity; i++) {
        atomic_init(buf->buf_mark + i, 0);
    }
    return buf;
}

void ring_buffer_free(ring_buffer * buf) {
    free(buf->buf);
    free(buf->buf_mark);
    free(buf);
}

int ring_buffer_read(ring_buffer * buf, void * dest) {
    long count = atomic_load_explicit(&buf->count, memory_order_relaxed);
    while (count > 0 && !atomic_compare_exchange_strong(&buf->count, &count, count - 1));
    // return when nothing to read
    if (count == 0) {
        return 0;
    }
    long idx = atomic_fetch_add_explicit(&buf->read_cursor, 1, memory_order_acq_rel) % buf->capacity;
    // loop until ready to read
    while (!atomic_load_explicit(buf->buf_mark + idx, memory_order_acquire));
    memcpy(dest, buf->buf + idx * buf->elem_size, buf->elem_size);
    // change state to ready-to-write
    atomic_store_explicit(buf->buf_mark + idx, 0, memory_order_release);
    return 1;
}

int ring_buffer_write(ring_buffer * buf, void * src) {
    long count = atomic_load_explicit(&buf->count, memory_order_relaxed);
    while (count < buf->capacity && !atomic_compare_exchange_strong(&buf->count, &count, count + 1));
    // return when it is full
    if (count == buf->capacity) {
        return 0;
    }
    long idx = atomic_fetch_add_explicit(&buf->write_cursor, 1, memory_order_acq_rel) % buf->capacity;
    // loop until ready to write
    while (atomic_load_explicit(buf->buf_mark + idx, memory_order_acquire));
    memcpy(buf->buf + idx * buf->elem_size, src, buf->elem_size);
    // change state to ready-to-read
    atomic_store_explicit(buf->buf_mark + idx, 1, memory_order_release);
    return 1;
}
