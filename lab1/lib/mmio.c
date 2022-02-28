#include "mailbox.h"

int mailbox_property(unsigned int identifier, unsigned int buffer_size, unsigned char ch){
  mbox[0] = 8 * 4; // buffer size in bytes
  mbox[1] = 0x00000000;
  mbox[2] = identifier; // tag identifier
  mbox[3] = buffer_size; // maximum of request and response value buffer's length.
  mbox[4] = 0x00000000;
  mbox[5] = 0; // value buffer
  mbox[6] = 0;
  if(mbox_call(ch)){
    return 1;
  }
  return 0; 
}
