#include "lib/ring_buffer.h"

ring_buffer* create_simple_ring_buf(size_t s){
    ring_buffer* tmp = (ring_buffer*)simple_malloc(sizeof(ring_buffer));
    tmp->buf = (uint8_t*)simple_malloc(s + 1);
    tmp->size = s;
    tmp->head = 0;
    tmp->tail = 0;
    tmp->count = 0;
    return tmp;
}

// This function can only be used after initializing mm 
ring_buffer* create_ring_buf(size_t s){
    ring_buffer* tmp = (ring_buffer*)kmalloc(sizeof(ring_buffer));
    tmp->buf = (uint8_t*)kmalloc(s + 1);
    tmp->size = s;
    tmp->head = 0;
    tmp->tail = 0;
    tmp->count = 0;
    return tmp;
}

void free_ring_buf(ring_buffer* rbuf){
    kfree(rbuf->buf);
    kfree(rbuf);
}

uint8_t ring_buf_is_empty(ring_buffer* rbuf){
    uint64_t daif;
    daif = local_irq_disable_save();
    if(rbuf->head == rbuf->tail){
        local_irq_restore(daif);
        return 1;
    }
    local_irq_restore(daif);
    return 0;
}

uint8_t ring_buf_is_full(ring_buffer* rbuf){
    uint64_t daif;
    if((rbuf->tail + 1) % (rbuf->size + 1) == rbuf->head){
        local_irq_disable_save(daif);
        return 1;
    }
    local_irq_restore(daif);
    return 0;
}

size_t ring_buf_write(ring_buffer* rbuf, uint8_t* data, size_t s){
    size_t count = 0;
    uint64_t daif;

    daif = local_irq_disable_save();
    while(!ring_buf_is_full(rbuf) && count < s){
        rbuf->buf[rbuf->tail] = data[count];

        rbuf->tail = (rbuf->tail + 1) % (rbuf->size + 1);
        count = count + 1;
    }
    rbuf->count += count;
    local_irq_restore(daif);
    return count;
}

size_t ring_buf_write_unsafe(ring_buffer* rbuf, uint8_t* data, size_t s){
    size_t count = 0;

    while(!ring_buf_is_full(rbuf) && count < s){
        rbuf->buf[rbuf->tail] = data[count];

        rbuf->tail = (rbuf->tail + 1) % (rbuf->size + 1);
        count = count + 1;
    }
    rbuf->count += count;
    return count;
}

size_t ring_buf_read(ring_buffer* rbuf, uint8_t* data, size_t s){
    size_t count = 0;
    uint64_t daif;

    daif = local_irq_disable_save();
    while(!ring_buf_is_empty(rbuf) && count < s){
        data[count] = rbuf->buf[rbuf->head];

        rbuf->head = (rbuf->head + 1) % (rbuf->size + 1);
        count = count + 1;
    }
    rbuf->count -= count;
    local_irq_restore(daif);
    return count;
}

size_t ring_buf_read_unsafe(ring_buffer* rbuf, uint8_t* data, size_t s){
    size_t count = 0;

    while(!ring_buf_is_empty(rbuf) && count < s){
        data[count] = rbuf->buf[rbuf->head];

        rbuf->head = (rbuf->head + 1) % (rbuf->size + 1);
        count = count + 1;
    }
    rbuf->count -= count;
    return count;
}

size_t ring_buf_get_len(ring_buffer* rbuf){
    uint64_t daif;
    size_t ret;

    daif = local_irq_disable_save();
    ret = rbuf->count;
    local_irq_restore(daif);

    return ret;
}
