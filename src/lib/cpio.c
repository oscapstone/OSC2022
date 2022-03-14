#include "cpio.h"
#include "my_math.h"
#include "my_string.h"
#include "uart.h"

#define NULL ((void *)0)
unsigned long CPIO_BASE;


static unsigned long cpio_len_next(unsigned long len, const void *prev, const void *next)
{
    unsigned long diff = (unsigned long)(next - prev);
    if (len < diff) {
        return 0;
    }
    return len;
}

unsigned long cpio_align(unsigned long n)
{
    return (n + CPIO_ALIGNMENT - 1) & (~(CPIO_ALIGNMENT - 1));
}

int cpio_parse_header(struct cpio_header *archive,
        const char **filename, unsigned long *_filesize, void **data,
        struct cpio_header **next)
{
    unsigned long filesize;
    /* Ensure magic header exists. */
    if (strncmp(archive->c_magic, CPIO_HEADER_MAGIC,
                sizeof(archive->c_magic)) != 0)
        return -1;

    /* Get filename and file size. */
    filesize = htouln(archive->c_filesize, sizeof(archive->c_filesize));
    *filename = ((char *)archive) + sizeof(struct cpio_header);

    /* Ensure filename is not the trailer indicating EOF. */
    if (strncmp(*filename, CPIO_FOOTER_MAGIC, sizeof(CPIO_FOOTER_MAGIC)) == 0)
        return 1;

    /* Find offset to data. */
    unsigned long filename_length = htouln(archive->c_namesize,
            sizeof(archive->c_namesize));
    *data = (void *)cpio_align(((unsigned long)archive)
            + sizeof(struct cpio_header) + filename_length);
    *next = (struct cpio_header *)cpio_align(((unsigned long)*data) + filesize);
    if(_filesize){
        *_filesize = filesize;
    }
    return 0;
}

/*
 * Get the location of the data in the n'th entry in the given archive file.
 *
 * We also return a pointer to the name of the file (not NUL terminated).
 *
 * Return NULL if the n'th entry doesn't exist.
 *
 * Runs in O(n) time.
 */
void *cpio_get_entry(void *archive, int n, const char **name, unsigned long *size)
{
    int i;
    struct cpio_header *header = archive;
    void *result = NULL;

    /* Find n'th entry. */
    for (i = 0; i <= n; i++) {
        struct cpio_header *next;
        int error = cpio_parse_header(header, name, size, &result, &next);
        if (error)
            return NULL;
        header = next;
    }

    return result;
}

/*
 * Find the location and size of the file named "name" in the given 'cpio'
 * archive.
 *
 * Return NULL if the entry doesn't exist.
 *
 * Runs in O(n) time.
 */
void *cpio_get_file(void *archive, const char *name, unsigned long *size)
{
    struct cpio_header *header = archive;

    /* Find n'th entry. */
    while (1) {
        struct cpio_header *next;
        void *result;
        const char *current_filename;

        int error = cpio_parse_header(header, &current_filename,
                size, &result, &next);
        if (error)
            return NULL;
        if (strncmp(current_filename, name, -1) == 0)
            return result;
        header = next;
    }
}

int cpio_info(void *archive, struct cpio_info *info) {
    struct cpio_header *header, *next;
    const char *current_filename;
    void *result;
    int error;
    unsigned long size, current_path_sz;

    if (info == NULL) return 1;
    info->file_count = 0;
    info->max_path_sz = 0;

    header = archive;
    while (1) {
        error = cpio_parse_header(header, &current_filename, &size,
                &result, &next);
        if (error == -1) {
            return error;
        } else if (error == 1) {
            /* EOF */
            return 0;
        }
        info->file_count++;
        header = next;

        // Check if this is the maximum file path size.
        current_path_sz = strlen(current_filename);
        if (current_path_sz > info->max_path_sz) {
            info->max_path_sz = current_path_sz;    
        }
    }

    return 0;
}

void cpio_ls() {
    void *archive = (void *)CPIO_BASE;
    const char *current_filename;
    struct cpio_header *header, *next;
    void *result;
    int error;
    unsigned long i, size;

    header = archive;
    for (;;) {
        error = cpio_parse_header(header, &current_filename, &size,
                &result, &next);
        // Break on an error or nothing left to read.
        if (error) break;
        uart_printf("%s\n", current_filename);
        header = next;
    }
}

int cpio_cat(const char *name) {
    void *archive = (void *)CPIO_BASE;
    const char *current_filename;
    struct cpio_header *header, *next;
    void *result;
    int error;
    unsigned long i, size;

    header = archive;
    for (;;) {
        error = cpio_parse_header(header, &current_filename, &size,
                &result, &next);
        // Break on an error or nothing left to read.
        if (error) break;
        if (strcmp(current_filename, name) == 0)
        {
            char *tmp = (char *)result;
            for (int i = 0; i < size; ++i)
            {
                uart_send(tmp[i]);
                if (tmp[i] == '\n')
                    uart_send('\r');
            }
            return 1;
        }
        header = next;
    }
    return 0;
}