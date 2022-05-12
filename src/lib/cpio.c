#include "./include/cpio.h"

int cpio_header_parser(cpio_new_header *header, char **file_name, unsigned long *file_size, char **data, cpio_new_header **next_header) {
    
    // check the magic word
    if (strncmp(header->c_magic, CPIO_MAGIC_NUM, 6) != 0)
        return -1;
    
    // get file size and pointer of file name
    *file_size = hexStr2int(header->c_filesize, 8); 
    *file_name = ((char *)header) + HEADER_SIZE;

    // check if it is the end of the cpio archieve
    if (!strncmp(*file_name, END_OF_CPIO, sizeof(END_OF_CPIO)))
        return 1;
    
    unsigned long file_name_size = hexStr2int(header->c_namesize, 8);
    unsigned long header_name_size = HEADER_SIZE + file_name_size;
    // align by 4. The pathname is followed by NUL bytes so that the total size of the fixed header plus pathname is a multiple of four
    unsigned long data_ptr = (unsigned long)header + header_name_size;
    data_ptr = align_by_4(data_ptr);
    // get the data pointer
    *data = (char *)data_ptr;

    // align by 4 again.
    data_ptr = (unsigned long)*data + *file_size;
    data_ptr = align_by_4(data_ptr);
    // get next header pointer
    *next_header = (cpio_new_header *)data_ptr;

    return 0;
}

void cpio_ls(cpio_new_header *header) {
    char *file_name;
    unsigned long file_size;
    char *data;
    cpio_new_header *cur_header = header;
    cpio_new_header *nxt_header;
    int header_info;

    // read content in cpio archieve
    while (1) {
        header_info = cpio_header_parser(cur_header, &file_name, &file_size, &data, &nxt_header);
        if (header_info) {
            break;
        }
        uart_puts(file_name);
        uart_puts("\r\n");
        cur_header = nxt_header;
    }
    return;
}

void cpio_cat(cpio_new_header *header, char *input) {
    char *file_name;
    int header_info = 0;
    unsigned long file_size;
    char *data;
    cpio_new_header *cur_header = header;
    cpio_new_header *nxt_header;
    while (1) 
    {
        header_info = cpio_header_parser(cur_header, &file_name, &file_size, &data, &nxt_header);
        if (header_info) {
            break;
        }
        if (strcmp(file_name, input) == 0) {
            for (int i = 0; i < file_size; i++) {
                uart_send(data[i]);
                if (data[i] == '\n') {
                    uart_send('\r');
                }
            }
            return;
        }
        cur_header = nxt_header;
    }
    uart_puts("no such file");
    return;
}

void *cpio_load (cpio_new_header *header, char *name) {
    char *file_name;
    int header_info = 0;
    unsigned long file_size;
    char *data;
    cpio_new_header *cur_header = header;
    cpio_new_header *nxt_header;
    
    char *prog_base;

    while (1) 
    {
        header_info = cpio_header_parser(cur_header, &file_name, &file_size, &data, &nxt_header);
        if (header_info) break;

        if (strcmp(name, file_name) == 0) 
        {
            prog_base = malloc(file_size);

            for (int i = 0; i < file_size; i++) 
            {
                prog_base[i] = data[i];
            }

            return (void *) prog_base;
        } 
        cur_header = nxt_header;
    }

    return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////
// int cpio_load_user_program_and_get_size(char* name, unsigned long long load_addr)
// {
//     char *file_name;
//     int header_info = 0;
//     unsigned long file_size;
//     char *data;
//     cpio_new_header *header = CPIO_BASE;
//     cpio_new_header *cur_header = header;
//     cpio_new_header *nxt_header;
    
//     while (1) 
//     {
//         header_info = cpio_header_parser(cur_header, &file_name, &file_size, &data, &nxt_header);
//         if (header_info) break;

//         if (strcmp(name, file_name) == 0) 
//         {
//             char* target = (char*)load_addr;
//             char* uaer_program = cur_header;
//             for (int i = 0; i < file_size; i++)
// 				{
// 					*target = *uaer_program;
// 					target++;
// 					uaer_program++;
// 				}
				
// 				return file_size;
//         } 
//         cur_header = nxt_header;
//     }
//     return 0;
// }