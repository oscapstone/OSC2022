#ifndef _RING_BUFFER_H_ 
#define _RING_BUFFER_H_
#include "types.h"

typedef struct{
    size_t size;
    uint64_t head;
    uint64_t tail;
    uint8_t* buf;
    size_t count;
}ring_buffer;

extern ring_buffer* create_ring_buf(size_t);
extern size_t ring_buf_write(ring_buffer*, uint8_t*, size_t);
extern size_t ring_buf_read(ring_buffer*, uint8_t*, size_t);
extern uint8_t ring_buf_is_full(ring_buffer*);
extern uint8_t ring_buf_is_empty(ring_buffer*);
extern size_t get_ring_buf_len(ring_buffer*);
#endif
