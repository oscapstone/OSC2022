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
#define BUF_SIZE    65536


void do_exit(int fd, int res)
{
    // close FD
    if (fd != -1)
        close(fd);
    // restore settings for STDIN_FILENO
    // if (isatty(STDIN_FILENO))
    // {
    //     tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    // }
    exit(res);
}



int set_interface_attribs (int fd, int speed, int parity) {

        struct termios tty;

        if (fd == -1)
        {
            // failed to open
            return -1;
        }
        // must be a tty
        if (!isatty(fd))
        {
            fprintf(stderr, "/dev/ttyUSB0 is not a tty\n");
            do_exit(fd, EXIT_FAILURE);
        }


        if (tcgetattr (fd, &tty) != 0)
        {
                printf ("error %d from tcgetattr", errno);
                return -1;
        }

       

        // tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag = 0;         // disable break processing
        // tty.c_iflag = 0;
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 0;            // 0.5 seconds read timeout

        // tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        // tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
        //                                 // enable reading
        // tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        // tty.c_cflag |= parity;
        // tty.c_cflag &= ~CSTOPB;  // one stop bits
        // tty.c_cflag &= ~CRTSCTS;

        tty.c_cflag = CS8 | CREAD | CLOCAL;

        // cfsetospeed (&tty, speed);
        // cfsetispeed (&tty, speed);

        if ((cfsetispeed(&tty, B115200) < 0) ||
            (cfsetospeed(&tty, B115200) < 0))
        {
            perror("Failed to set baud-rate");
            do_exit(fd, EXIT_FAILURE);
        }

        if (tcsetattr (fd, TCSAFLUSH, &tty) != 0)
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
    dst_fd = open(portname, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(dst_fd < 0) {
        printf ("error %d opening %s: %s", errno, portname, strerror (errno));
        return -1;
    }

    if( set_interface_attribs(dst_fd, B115200, 0) == -1) {
        printf("The setting occurred error\n");
    }
    // set_blocking(dst_fd, 1);


    struct stat filestat;


    if(stat(kernelpath, &filestat)) {
        printf("Error %d opening %s: %s\n", errno, kernelpath, strerror(errno));
        return -1;
    }

    if(!S_ISREG(filestat.st_mode)) {
        printf("Error: this is not a regular file\n");
        return -1;
    }else {
        src_fd = open(kernelpath, O_RDONLY);
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
    ssize_t pos;
    uint32_t prev_cmd = 0; 
    char* p;

    //* set fd blocking
    if (fcntl(dst_fd, F_SETFL, 0) == -1)
    {
        perror("fcntl()");
        do_exit(dst_fd, EXIT_FAILURE);
    }


    //* send 4 byte secret
    while(1) {
        cmd = SEND_REQ;
        p = (char*)&cmd;
        pos = 0;
        tcflush(dst_fd, TCOFLUSH);
        while(pos < 4) {
            ssize_t len = write(dst_fd, &p[pos], 1);
            usleep(100);
            if (len == -1)
            {
                perror("write()");
                do_exit(dst_fd, EXIT_FAILURE);
            }
            pos += len;
        }
        printf("Send 0x%x\n", cmd);

        usleep(100);
        p = (char*)&cmd;
        pos = 0;
        while(pos < 4) {
            ssize_t len = read(dst_fd, &p[pos], 4 - pos);
            if (len == -1)
            {
                perror("read()");
                do_exit(dst_fd, EXIT_FAILURE);
            }
            pos += len;
        }
        if(cmd == ACK)  
            break;

        // if(prev_cmd != cmd)
        printf("Read [%08x]\n", cmd);

        prev_cmd = cmd;
    }
    // p = (char*)&cmd;
    // pos = 0;
    // while(pos < 4) {
    //     ssize_t len = write(dst_fd, &p[pos], 1);
    //     usleep(100);
    //     if (len == -1)
    //     {
    //         perror("write()");
    //         do_exit(dst_fd, EXIT_FAILURE);
    //     }
    //     pos += len;
    // }

    // printf("Send 0x%x\n", cmd);
    // // _write(dst_fd, &cmd, sizeof(cmd));
    
    // p = (char*)&cmd;
    
    // pos = 0;

    // while(1) {
    //     while(pos < 4) {
    //         ssize_t len = read(dst_fd, &p[pos], 4 - pos);
    //         if (len == -1)
    //         {
    //             perror("read()");
    //             do_exit(dst_fd, EXIT_FAILURE);
    //         }
    //         pos += len;
    //     }
    //     if(cmd == ACK)  
    //         break;

    //     if(prev_cmd != cmd)
    //         printf("Read [%08x]\n", cmd);

    //     prev_cmd = cmd;
    // }
    

    // while(1) {
    //     int n = _recv(dst_fd, &cmd, sizeof(cmd)); //* recv 4 B
    //     printf("[%d] recv : 0x%x\n", n, cmd);
    //     if(cmd == ACK)
    //         break;
    // }

    //* sent size in Byte
    ksize = finfo->st_size;
    p = (char*)&ksize;
    pos = 0;
    usleep(100);
    while(pos < 8) {
        ssize_t len = write(dst_fd, &p[pos], 1);
        usleep(100);
        if (len == -1)
        {
            perror("write()");
            do_exit(dst_fd, EXIT_FAILURE);
        }
        pos += len;
    }
    printf("file size %08x\n", (uint32_t)ksize);
    // _write(dst_fd, &ksize, sizeof(ksize));
    usleep(100);

    ksize = 0;
    p = (char*)&ksize;
    pos = 0;
    usleep(100);
    while(pos < 8) {
        ssize_t len = read(dst_fd, &p[pos], 8 - pos);
        usleep(100);
        if (len == -1)
        {
            perror("write()");
            do_exit(dst_fd, EXIT_FAILURE);
        }
        pos += len;
    }
    printf("file size %08x\n", (uint32_t)ksize);


    lseek(src_fd, 0, SEEK_SET);

    // uint32_t buf;

    // uint32_t addr = 0;
    int sent = 0;
    int done = 0;
    while(!done) {
        char buf[BUF_SIZE];
        ssize_t pos = 0;
        // int wrsize = ksize < 4 ? ksize : 4;
        int rdsize = read(src_fd, buf, BUF_SIZE);

        switch(rdsize) {
        case -1: 
            perror("read()");
            do_exit(dst_fd, EXIT_FAILURE);
        case 0:
            done = 1;
            
        }
        while(rdsize > 0) {
            ssize_t len2 = write(dst_fd, &buf[pos], 4);
            usleep(100);
            if(len2 == -1) {
                perror("write()");
                do_exit(dst_fd, EXIT_FAILURE);
            }
            rdsize -= len2;
            pos += len2;
            sent += len2;
        }

        printf("sent : [%08x] Bytes\n", sent);
    }

    cmd = MAGIC_CMD;
    p = (char*)&cmd;
    pos = 0;
    // tcflush(dst_fd, TCOFLUSH);
    while(pos < 4) {
        ssize_t len = write(dst_fd, &p[pos], 4 - pos);
        if (len == -1)
        {
            perror("write()");
            do_exit(dst_fd, EXIT_FAILURE);
        }
        pos += len;
    }
    printf("Written to %08x\n", cmd);
    // _write(dst_fd, &cmd, sizeof(cmd));

    cmd = 0;
    p = (char*)&cmd;
    pos = 0;
    while(pos < 4) {
        ssize_t len = read(dst_fd, &p[pos], 4 - pos);
        if (len == -1)
        {
            perror("read()");
            do_exit(dst_fd, EXIT_FAILURE);
        }
        pos += len;
    }
    // _recv(dst_fd, &cmd, sizeof(cmd));
    if(cmd == ACK) {
        printf("Successful\n");
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