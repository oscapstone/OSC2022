#include "string.h"
#include "memory.h"
#include "uart.h"
#include "type.h"
#include "utils.h"

char* extract_section(char* context, char* p, char* address, int len) {
    int i;
    for(i = 0; (*p) != 0x00 && i < len; p++, i++) { // NULL or exceed len
        context[i] = *p;
    }
    context[i] = '\0';
    if((*p) == 0x00) { //4 bytes padding if necessary 
        p += 4 - ((p - address) % 4);
    }
    return p;
}

void extract_cpio(char* address, int ls, int cat, char* name) {
    char filename[256];
    char content[1024];
    char* p = address;
    uint64 namesize, filesize;
        
    do {
        filesize = str2num(p + 54, 8);
        // namesize = str2num(p + 94, 8);
        // uart_puts("file size: "); uart_num(filesize); uart_newline();
        // uart_puts("name size: "); uart_num(namesize); uart_newline();
        p = extract_section(filename, p + 110, address, 1000); // read filename
        
        if(strcmp(filename, "TRAILER!!!")) { // ending string
            break;
        }

        if(ls == 1) {   // show filename
            uart_puts("File: ");
            uart_puts(filename);
            uart_newline();
        }

        if(strcmp(filename, ".")) { // always the first file without any content
            continue;
        }

        // get file content
        p = extract_section(content, p, address, filesize);
        // show content
        if(cat == 1 && strcmp(name, filename)) {
            uart_puts("Filename: ");
            uart_puts(filename);
            uart_newline();
            uart_puts("Content: ");
            uart_puts(content);
            uart_newline();
        }
    } while(strcmp(filename, "TRAILER!!!") == 0);
}