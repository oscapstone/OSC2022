#define MBOX_REQUEST            0
#define MBOX_CH_PROP            8//CPU->GPU
#define MBOX_TAG_GETSERIAL      0x10004
#define MBOX_TAG_GETREVISION    0x00010002
#define MBOX_TAG_GETVCMEM       0x00010006
#define MBOX_TAG_LAST           0
#define TAG_REQUEST_CODE        0

#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MBOX_EMPTY      0x40000000
#define MBOX_FULL       0x80000000
#define MBOX_RESPONSE   0x80000000
unsigned int mbox[36];
int getBoardRevision(unsigned int* dst);
int getpid();
int uart_read(char* buf,int size);
int uart_write(char* buf,int size);
unsigned int uart_printf(char* fmt,...);
int exec(char* name,char** argv);
void exit();
int fork();
int mbox_call(unsigned char ch, unsigned int *mbox);