#include "exception.h"
#include "uart.h"
#include "utils.h"
#include "timer.h"
#include "string.h"
#include "thread.h"
#include "printf.h"
#include "mbox.h"
#include "vfs.h"
#include "device.h"

int count = 0;

void enable_interrupt() { asm volatile("msr DAIFClr, 0xf"); }

void disable_interrupt() { asm volatile("msr DAIFSet, 0xf"); }

void sync_handler_currentEL_ELx() {
  printf("[sync_handler_currentEL_ELx]\n");

  uint64_t spsr_el1, elr_el1, esr_el1;
  asm volatile("mrs %0, spsr_el1" : "=r"(spsr_el1));
  asm volatile("mrs %0, elr_el1" : "=r"(elr_el1));
  asm volatile("mrs %0, esr_el1" : "=r"(esr_el1));
  while(1);
}

void sync_handler_lowerEL_64(uint64_t sp) {
  // printf("sync_handler_lowerEL_64 sp : %x\n",sp);
  uint64_t spsr_el1, elr_el1, esr_el1;
  asm volatile("mrs %0, spsr_el1" : "=r"(spsr_el1));
  asm volatile("mrs %0, elr_el1" : "=r"(elr_el1));
  asm volatile("mrs %0, esr_el1" : "=r"(esr_el1));
  // printf("sync, SPSR_EL1: 0x%08x\n", spsr_el1);
  // printf("ELR_EL1: 0x%08x\n", elr_el1);
  // printf("ESR_EL1: 0x%08x\n", esr_el1);

  uint32_t ec = (esr_el1 >> 26) & 0x3f;
  // printf("EC: %x\n", ec);
  if (ec == 0b010101) {  // SVC instruction
    // printf("pid = %d, ",get_current()->pid);
    uint64_t iss;
    asm volatile("mov %0, x8" : "=r"(iss));
    // printf("syscall number: %d\n", iss);
    trap_frame_t *trap_frame = (trap_frame_t *)sp;

    if (iss == 0) {  // getpid
      uint32_t pid = get_current()->pid;
      trap_frame->x[0] = pid;
    } else if (iss == 1) {  // uartread
      // printf("[read]\n");
      disable_uart_interrupt();
      enable_interrupt();
      char *str = (char *)(trap_frame->x[0]);
      uint32_t size = (uint32_t)(trap_frame->x[1]);
      size = uart_gets(str, size);
      trap_frame->x[0] = size;
    } else if (iss == 2) {  // uartwrite
      // printf("[write]\n");
      char *str = (char *)(trap_frame->x[0]);
      trap_frame->x[0] = uart_write(str,trap_frame->x[1]);
    } else if (iss == 3) {  // exec
      const char *program_name = (const char *)trap_frame->x[0];
      const char **argv = (const char **)trap_frame->x[1];
      exec(program_name, argv);
    } else if (iss == 4) {  // fork
      // printf("[fork]\n");
      fork(sp);
    } else if (iss == 5) {  // exit
      exit();
    } else if (iss == 6) {  // mbox_call
      printf("\n\n\n[mbox_call]\n");
      trap_frame->x[0] = mbox_call(trap_frame->x[0],(unsigned int *)trap_frame->x[1]);
    } else if (iss == 7) {  // kill
      kill((int)trap_frame->x[0]);
    } //vfs
      else if (iss == 11) {  // open
      printf("[open]%s\n",(const char *)trap_frame->x[0]);
      struct file* file = vfs_open((const char *)trap_frame->x[0],trap_frame->x[1]);
      int fd = thread_register_fd(file);
      printf("[open]fd :%d\n",fd);

      trap_frame->x[0] = fd;

    } else if (iss == 12) {  // close
      printf("[close]fd =%d\n",trap_frame->x[0]);
      struct file* file = thread_get_file(trap_frame->x[0]);
      vfs_close(file);

    } else if (iss == 13) {  // write
      // remember to return read size or error code
      // printf("[write]\n");
      // printf("[write]fd =%d\n",trap_frame->x[0]);
      // printf("[write]write_buf =%s\n",trap_frame->x[1]);
      // printf("[write]size =%d\n",trap_frame->x[2]);
      struct file* file = thread_get_file(trap_frame->x[0]);
      trap_frame->x[0] = vfs_write(file, (const void *)trap_frame->x[1], trap_frame->x[2]);

    } else if (iss == 14) {  // read
      // remember to return read size or error code
      printf("[read]\n");
      printf("[read]id =%d\n",trap_frame->x[0]);
      printf("[read]read_buf =%s\n",trap_frame->x[1]);
      printf("[read]size =%d\n",trap_frame->x[2]);
      struct file* file = thread_get_file(trap_frame->x[0]);
      trap_frame->x[0] = vfs_read(file, (void *)trap_frame->x[1], trap_frame->x[2]);
      printf("[read][after] read_buf =%s\n",trap_frame->x[1]);

    } else if (iss == 15) {  // mkdir
      // you can ignore mode, since there is no access control
      printf("[mkdir]%s\n",(const char *)trap_frame->x[0]);
      trap_frame->x[0] = vfs_mkdir((const char *)trap_frame->x[0]);

    } else if (iss == 16) {  // mount
      // you can ignore arguments other than target (where to mount) and filesystem (fs name)
      const char *device = (const char *)trap_frame->x[0];
      const char *mountpoint = (const char *)trap_frame->x[1];
      const char *filesystem = (const char *)trap_frame->x[2];
      printf("[mount]mountpoint %s\n", mountpoint);
      printf("[mount]filesystem %s\n", filesystem);

      int result = vfs_mount(device, mountpoint, filesystem);
      trap_frame->x[0] = result;

    } else if (iss == 17) {  // chdir
      const char *pathname = (const char *)trap_frame->x[0];
      printf("[chdir]%s\n", pathname);

      int result = vfs_chdir(pathname);
      trap_frame->x[0] = result;
    } else if (iss == 18) {  // lseek64
      // printf("[lseek64]\n");
      // printf("[lseek64]fd = %d\n", trap_frame->x[0]);
      // printf("[lseek64]offset = %ld\n", (long) trap_frame->x[1]);
      // printf("[lseek64]whence = %d\n", trap_frame->x[2]);
      trap_frame->x[0] = lseek64(trap_frame->x[0], trap_frame->x[1], trap_frame->x[2]);
    } else if (iss == 19) {  // ioctl
      printf("[ioctl]\n");
      printf("[ioctl]fd = %d\n", trap_frame->x[0]);
      printf("[ioctl]request = %ld\n", (long) trap_frame->x[1]);
      trap_frame->x[0] = ioctl(trap_frame->x[0], trap_frame->x[1], (struct framebuffer_info*)trap_frame->x[2]);
    } 
  }
}


void irq_handler_currentEL_ELx() {
  // printf("====irq_handler_currentEL_ELx=====\n");

  disable_interrupt();
  uint32_t is_uart = (*IRQ_PENDING_1 & AUX_IRQ);
  uint32_t is_core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);

  if (is_uart) {
    uart_handler();
  } else if (is_core_timer) {
    core_timer_handler_lowerEL_64();
  }
  enable_interrupt();
}

void irq_handler_lowerEL_64() {
  // printf("====irq_handler_lowerEL_64=====\n");
  disable_interrupt();
  uint32_t is_uart = (*IRQ_PENDING_1 & AUX_IRQ);
  uint32_t is_core_timer = (*CORE0_INTERRUPT_SOURCE & 0x2);

  if (is_uart) {
    uart_handler();
  } else if (is_core_timer) {
    core_timer_handler_lowerEL_64();
  }
  enable_interrupt();
}


void default_handler() { uart_puts("===== default handler =====\n"); }
