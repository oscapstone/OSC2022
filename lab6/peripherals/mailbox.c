#include "peripherals/mailbox.h"

static uint32_t MBox_buf[4096] __attribute__((aligned(16)));

static uint32_t MBox_read(int channel){
    int ch, data;

    LOG("Enter");
    MBox_register* mbox_reg = (MBox_register*)MBOX_REG;
    while(mbox_reg->status & MBOX_STATUS_EMPTY_MASK);
   
    data = mbox_reg->read;
    ch = data & MBOX_WRITE_CHANNEL_MASK; 
    if(ch == channel){
        LOG("MBox read address: %p", data & MBOX_READ_ADDR_MASK);
        return (data & MBOX_READ_ADDR_MASK);
    }
    LOG("Leave");
    return 0;
}

static void MBox_write(uint32_t data, int channel){
    LOG("Enter");
    MBox_register* mbox_reg = (MBox_register*)MBOX_REG;
    while(mbox_reg->status & MBOX_STATUS_FULL_MASK);

    mbox_reg->write = data | channel;
    LOG("Leave");
}

static uint32_t MBox_process(uint8_t* tags, size_t tags_size){
    LOG("Enter");
    MBox_buffer* mbuf = (MBox_buffer*)MBox_buf;
    // set buffer header
    mbuf->buf_size = tags_size + 12; 
    mbuf->code = MBOX_STATUS_REQUEST;

    // copy tags to buffer
    memcpy(mbuf->buf, tags, tags_size);
    uint32_t *pbuf = (uint32_t*)mbuf;
    for(int i = 0 ; i < 10 ; i++){
        LOG("%p: %p", &pbuf[i], pbuf[i]);
    }
    
    LOG("mbuf->buf: %p", mbuf->buf);
    LOG("mbuf->buf + tags_size: %p", mbuf->buf + tags_size);
    // end of MBox_buf
    memset(mbuf->buf + tags_size, 0, 4);
   
    pbuf = (uint32_t*)mbuf;
    for(int i = 0 ; i < 10 ; i++){
        LOG("%p: %p", &pbuf[i], pbuf[i]);
    }

    // write to mailbox
    MBox_write((uint32_t)(uint64_t)virt_to_phys(MBox_buf), MBOX_CHANNEL_PROPERTY_TAGS);
    
    // read from mailbox
    uint32_t result = MBox_read(MBOX_CHANNEL_PROPERTY_TAGS);


    LOG("Leave");
    return result;
}
void MBox_get_board_revision(uint32_t* ret){
    LOG("Enter");
    // create tags
    uint8_t buf[sizeof(MBox_tag) + 4];
    MBox_tag* tag = (MBox_tag*)buf;
    tag->id = MBOX_GET_BOARD_REVISION;
    tag->value_size = 4;
    tag->code = MBOX_TAG_REQUEST;
    ((uint32_t*)tag->values)[0] = 0;
    
    // process tags
    MBox_buffer* mbuf = (MBox_buffer*)phys_to_virt(MBox_process(buf, sizeof(MBox_tag) + 4));
	
    // precoess response tags
    tag = (MBox_tag*)mbuf->buf;
    uint32_t *pbuf = (uint32_t*)mbuf;
    for(int i = 0 ; i < 10 ; i++){
        LOG("%p: %p", &pbuf[i], pbuf[i]);
    }

    *ret = ((uint32_t*)tag->values)[0];
    LOG("Leave");
}
void MBox_get_arm_memory(uint32_t* ret){
    LOG("Enter");
    // create tags
    uint8_t buf[sizeof(MBox_tag) + 8];
    MBox_tag* tag = (MBox_tag*)buf;
    tag->id = MBOX_GET_ARM_MEMORY;
    tag->value_size = 8;
    tag->code = MBOX_TAG_REQUEST;
    ((uint32_t*)tag->values)[0] = 0;
    ((uint32_t*)tag->values)[1] = 0;
    
    // process tags
    MBox_buffer* mbuf = (MBox_buffer*)phys_to_virt(MBox_process(buf, sizeof(MBox_tag) + 8));

    // precoess response tags
    tag = (MBox_tag*)mbuf->buf;
    uint32_t *pbuf = (uint32_t*)mbuf;
    for(int i = 0 ; i < 10 ; i++){
        LOG("%p: %p", &pbuf[i], pbuf[i]);
    }

    ret[0] = ((uint32_t*)tag->values)[0];
    ret[1] = ((uint32_t*)tag->values)[1];
    LOG("Leave");
}

int Mbox_call(uint32_t* mbox, uint8_t ch) {
    MBox_register* mbox_reg = (MBox_register*)MBOX_REG;
    while(mbox_reg->status & MBOX_STATUS_FULL_MASK){
        preempt_schedule(); 
    }

    mbox_reg->write = ((uint32_t)(uint64_t)mbox & ~0xF) | ch;

    while (mbox_reg->status & MBOX_STATUS_EMPTY_MASK){
        preempt_schedule(); 
    }

    return mbox_reg->read == (((uint32_t)(uint64_t)mbox & ~0xF) | ch); 
}
int sys_mbox_call(uint8_t ch, uint32_t *mbox){
    return Mbox_call(mbox, ch);
}
