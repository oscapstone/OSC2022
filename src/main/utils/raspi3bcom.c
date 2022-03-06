#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAGIC_CMD   0xdeadbeef

#define SEND_REQ    (MAGIC_CMD ^ 0x1)
#define ACK         (MAGIC_CMD ^ 0x2)

int set_interface_attribs (int fd, int speed, int parity) {
        struct termios tty;
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;  // one stop bits
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                printf ("error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void set_blocking (int fd, int should_block) {
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                printf ("error %d setting term attributes", errno);
}

int _recv(int fd, char* buf, size_t size);
int _write(int fd, char* buf, size_t size);

int send_img(int dst_fd, int src_fd, struct stat* finfo);
void dump_32b(uint32_t val);


int main(int argc, char* argv[]) {


    char* portname = argv[1];
    char* kernelpath = argv[2];

    int src_fd, dst_fd;

    printf("Open %s\n", portname);
    dst_fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
    if(dst_fd < 0) {
        printf ("error %d opening %s: %s", errno, portname, strerror (errno));
        return -1;
    }

    set_interface_attribs(dst_fd, B115200, 0);
    set_blocking(dst_fd, 1);


    struct stat filestat;


    if(stat(kernelpath, &filestat)) {
        printf("Error %d opening %s: %s\n", errno, kernelpath, strerror(errno));
        return -1;
    }

    if(!S_ISREG(filestat.st_mode)) {
        printf("Error: this is not a regular file\n");
        return -1;
    }else {
        src_fd = open(kernelpath, O_RDONLY | O_SYNC);
        if(src_fd < 0) {
            printf ("error %d opening %s: %s", errno, kernelpath, strerror (errno));
            return -1;
        }
    }


    int ret = send_img(dst_fd, src_fd, &filestat);

    if(ret == 0)
        printf("The data is transmitting successfully\n");
    else
        printf("Failed\n");

    return 0;
}



int _recv(int fd, char* buf, size_t size) {

    int r = size;
    int n;
    int sum = 0;
    char* cur = buf;
    while(r) {
        n = read(fd, cur, size);
        cur += n;
        r -= n;
        sum += n;
    }

    return sum;

}

int _write(int fd, char* buf, size_t size) {


    int n;
    int sum = 0;
    if((n = write(fd, buf, size)) == -1)
        printf("Sent failed\n");
    if(tcdrain(fd) != 0) {
        printf("tcdrain() error\n");
    } else {
        sum += n;
    }

    return sum;

}

int send_img(int dst_fd, int src_fd, struct stat* finfo) {


    //* Send SEND_REQ cmd to bootloader

    uint32_t cmd = SEND_REQ;
    int64_t  ksize = 0;

    //* send 4 byte secret
    _write(dst_fd, &cmd, sizeof(cmd));
    printf("Send 0x%x\n", cmd);

    int n = 0;
    while(1) {
        n = _recv(dst_fd, &cmd, sizeof(cmd)); //* recv 4 B
        printf("[%d] recv : 0x%x\n", n, cmd);
        if(cmd == ACK)
            break;
    }

    //* sent size in Byte
    ksize = finfo->st_size;
    printf("file size %ld", ksize);
    _write(dst_fd, &ksize, sizeof(ksize));


    lseek(src_fd, 0, SEEK_SET);

    uint32_t buf;

    uint32_t addr = 0;
    while(ksize) {
        int wrsize = ksize < 4 ? ksize : 4;
        int rdsize = read(src_fd, &buf, sizeof(buf));
        wrsize = _write(dst_fd, &buf, sizeof(char) * rdsize);

        if(rdsize != wrsize) {
            printf("Sent is not sync\n");
            return -1;
        }
        printf("[%08x] ", addr);
        dump_32b(buf);
        printf(" re: %08x/%08x", ksize-rdsize, finfo->st_size);
        printf("\n");
        addr += wrsize;
        ksize -= rdsize;
    }

    cmd = MAGIC_CMD;
    _write(dst_fd, &cmd, sizeof(cmd));
    _recv(dst_fd, &cmd, sizeof(cmd));
    if(cmd == ACK) {
        return 0;
    }

    return -1;

}

void dump_32b(uint32_t val) {

    for(int i=0;i<32;i+=8) {
        printf("%02x", val & 0xff);
        val >>= 8;
    }
}