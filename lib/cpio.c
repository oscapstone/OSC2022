#include "string.h"
#include "memory.h"
#include "uart.h"
#include "type.h"
#include "utils.h"

unsigned int hex2num(char *s, unsigned int max_len) {
    unsigned int r = 0;

    for (unsigned int i = 0; i < max_len; i++) {
        r *= 16;
        if (s[i] >= '0' && s[i] <= '9') {
            r += s[i] - '0';
        }  else if (s[i] >= 'a' && s[i] <= 'f') {
            r += s[i] - 'a' + 10;
        }  else if (s[i] >= 'A' && s[i] <= 'F') {
            r += s[i] - 'A' + 10;
        } else {
            return r;
        }
    }
    return r;
}

char* extract_section(char* context, char* p, char* address, int len) {
    int i;
    for(i = 0; i < len; i++) { // NULL or exceed len
        context[i] = *p++;
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
        filesize = hex2num(p + 54, 8);
        namesize = hex2num(p + 94, 8);
        uart_puts("file size: "); uart_num(filesize); uart_newline();
        uart_puts("name size: "); uart_num(namesize); uart_newline();
        p = extract_section(filename, p + 110, address, namesize); // read filename
        
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

        
        // show content
        
        if(cat == 1 && strcmp(name, filename)) {
            // get file content
            p = extract_section(content, p, address, filesize);
            uart_puts("Filename: ");
            uart_puts(filename);
            uart_newline();
            uart_puts("Content: ");
            uart_puts(content);
            uart_newline();
        }
        else {
            p += filesize;
            if((*p) == 0x00) { //4 bytes padding if necessary 
                p += 4 - ((p - address) % 4);
            }
        }
        
    } while(strcmp(filename, "TRAILER!!!") == 0);
}

char* load_user_program(char* address, char* program_address, char* name) {
    char filename[256];
    char* p = address;
    uint64 filesize, namesize;
        
    do {
        filesize = hex2num(p + 54, 8);
        namesize = hex2num(p + 94, 8);
        p = extract_section(filename, p + 110, address, namesize); // read filename
        
        if(strcmp(filename, "TRAILER!!!")) { // ending string
            raiseError("Can't find the user program!!!");
        }
        else if(strcmp(filename, ".")) { // always the first file without any content
            continue;
        }
        else if(strcmp(filename, name)) {
            break;
        }
        else {
            p += filesize;
        }
    } while(strcmp(filename, "TRAILER!!!") == 0);

    // uart_puts("Get user program\n");
    return p;
}

uint64 get_program_size(char* address, char* name) {
    char filename[256];
    char* p = address;
    uint64 filesize, namesize;
        
    do {
        filesize = hex2num(p + 54, 8);
        namesize = hex2num(p + 94, 8);
        p = extract_section(filename, p + 110, address, namesize); // read filename
        
        if(strcmp(filename, "TRAILER!!!")) { // ending string
            raiseError("Can't find the user program!!!");
        }
        else if(strcmp(filename, ".")) { // always the first file without any content
            continue;
        }
        else if(strcmp(filename, name)) {
            return filesize;
        }
        else {
            p += filesize;
        }
    } while(strcmp(filename, "TRAILER!!!") == 0);

    
    return 0;
}