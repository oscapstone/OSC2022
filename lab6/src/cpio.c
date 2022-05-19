#include "cpio.h"
#include "string.h"
#include <stddef.h>

#define FILENAME_LEN 256
#define min(x, y) ((x) > (y) ? (y) : (x))

static uint32_t stoul(char *s, uint32_t len) {
  uint32_t val = 0;
  for (int slen = 0; slen < len && s[slen] != 0; slen++) {
    val <<= 4;
    if (s[slen] >= 'A' && s[slen] <= 'F') {
      val += s[slen] - 'A' + 10;
    } else if (s[slen] >= 'a' && s[slen] <= 'f') {
      val += s[slen] - 'a' + 10;
    } else if (s[slen] >= '0' && s[slen] <= '9') {
      val += s[slen] - '0';
    } else {
      return -1;
    }
  }
  return val;
}

inline static uint32_t cpio_filenamesize(struct cpio_newc_header *p_header) {
  return stoul(p_header->c_namesize, sizeof(p_header->c_namesize));
}

uint32_t cpio_filename(struct cpio_newc_header *p_header, char *buf,
                       uint32_t size) {
  uint64_t nsize = cpio_filenamesize(p_header);
  char *name = ((char*)p_header) + sizeof(struct cpio_newc_header);
  int i;
  for (i = 0; i < nsize && i < size; i++) {
    buf[i] = name[i];
  }
  if (i < size) buf[i++] = 0;
  return nsize;
}

uint32_t cpio_filesize(struct cpio_newc_header *p_header) {
  return stoul(p_header->c_filesize, sizeof(p_header->c_filesize));
}

uint32_t cpio_read(struct cpio_newc_header *p_header, uint32_t offset,
                   char *buf, uint32_t size) {
  uint32_t nsize = cpio_filenamesize(p_header);
  if ((nsize + sizeof(struct cpio_newc_header)) % 4 != 0) {
    nsize += 4 - ((nsize + sizeof(struct cpio_newc_header)) & 3);
  }
  uint32_t fsize = cpio_filesize(p_header) - offset;
  char *content = ((char*)p_header) + sizeof(struct cpio_newc_header) + nsize + offset;
  int i;
  for (i = 0; i < fsize && i < size; i++) {
    buf[i] = content[i];
  }
  return i;
}

struct cpio_newc_header *cpio_nextfile(struct cpio_newc_header* p_header) {
  uint32_t nsize = cpio_filenamesize(p_header);
  uint32_t fsize = cpio_filesize(p_header);
  if ((nsize + sizeof(struct cpio_newc_header)) % 4 != 0) {
    nsize += 4 - ((nsize + sizeof(struct cpio_newc_header)) & 3);
  }
  if (fsize % 4 != 0) {
    fsize += 4 - (fsize & 3);
  }
  struct cpio_newc_header* p_next = (struct cpio_newc_header*)
    (((char*)p_header)+sizeof(struct cpio_newc_header)+nsize+fsize);
  char fname[sizeof("TRAILER!!!")];
  cpio_filename(p_next, fname, sizeof(fname));
  if (strncmp(fname, "TRAILER!!!", sizeof(fname)) == 0) {
    return (struct cpio_newc_header*)0;
  }
  return p_next;
}

struct cpio_newc_header *cpio_first(uint64_t addr) {
  struct cpio_newc_header* p_header = (struct cpio_newc_header*)addr;
  char fname[sizeof("TRAILER!!!")];
  cpio_filename(p_header, fname, sizeof(fname));
  if (strncmp(fname, "TRAILER!!!", sizeof(fname)) == 0) {
    return (struct cpio_newc_header*)0;
  }
  return p_header;
}

struct cpio_newc_header* find_file(uint64_t initrd_addr, const char *filename, uint32_t namelen) {
  static char fname[FILENAME_LEN];
  struct cpio_newc_header *p_header = cpio_first(initrd_addr);
  while (p_header != NULL) {
    cpio_filename(p_header, fname, FILENAME_LEN);
    if (strncmp(fname, filename, min(FILENAME_LEN, namelen)) == 0) {
      return p_header;
    }
    p_header = cpio_nextfile(p_header);
  }
  return NULL;
}
