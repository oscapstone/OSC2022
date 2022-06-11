#ifndef _RING_BUFFER_H_ 
#define _RING_BUFFER_H_
#include "types.h"
#include "lib/simple_malloc.h"
#include "mm/slab.h"

typedef struct{
    size_t size;
    uint64_t head;
    uint64_t tail;
    uint8_t* buf;
    size_t count;
}ring_buffer;

extern ring_buffer* create_simple_ring_buf(size_t);

extern ring_buffer* create_ring_buf(size_t);
extern void free_ring_buf(ring_buffer*);
extern size_t ring_buf_write(ring_buffer*, uint8_t*, size_t);
extern size_t ring_buf_write_unsafe(ring_buffer*, uint8_t*, size_t);
extern size_t ring_buf_read(ring_buffer*, uint8_t*, size_t);
extern size_t ring_buf_read_unsafe(ring_buffer*, uint8_t*, size_t);
extern uint8_t ring_buf_is_full(ring_buffer*);
extern uint8_t ring_buf_is_empty(ring_buffer*);
extern size_t ring_buf_get_len(ring_buffer*);

#endif
