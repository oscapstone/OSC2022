#include "string.h"
#include "type.h"

int strcmp (const char* str1, const char* str2) {
    for(int i = 0; ; i++) {
        if(str1[i] != str2[i]) {
            return 0;
        }
        else if(str1[i] == '\0' && str2[i] == '\0') {
            return 1;
        }
    }
}

void strcpy(char* dst, const char* src) {
    while(*src)
    {
        *(dst++) = *(src++);
    }

    *(dst) = '\0';
}

int startwith(const char* str1, const char* str2) {
    for(int i = 0; ; i++) {
        if(str2[i] == '\0') {
            return 1;
        }
        else if(str1[i] != str2[i]) {
            return 0;
        }
    }
}

int strlen(const char* str1) {
    int len = 0;
    while(1) {
        if(str1[len] == '\0') {
            break;
        }
        
        len++;
    }
    return len;
}

unsigned int str2num(char* str, int len) {
    int num = 0;
    char c;

    while(len--) {
        c = *(str++);
        num = num * 10 + c - '0';
    } 

    return num;
}

char* find_token(const char* str, char sep)
{
    while(*(str++) != sep);
    return (char*)str;
}

void strcat(char* dst, const char* src) {
    strcpy(dst + strlen(dst), src);
}

char *get_dirname(char* dst, const char* src)
{
    int last_slash_idx = -1;
    int i = 0;
    for(; src[i] != '\0'; i++) {
        if(src[i] == '/') {
            last_slash_idx = i;
        }
        dst[i] = src[i];
    }

    if(last_slash_idx >= 0) {
        dst[last_slash_idx] = '\0';
        return (char*)src + last_slash_idx + 1;
    }
    else {
        dst[i] = '\0';
        return 0x0000;
    }
}

char *get_rootname(char* dst, const char* src)
{
    if(*src == '/') src++;
    while(*src != '/' && *src != '\0') *(dst++) = *(src++);
    
    *dst = '\0';
    if(*src == '/') return (char*)src + 1;
    else return NULL;
}

void get_abspath(char *abs_path, char *path, const char *curr_path) {
    char tmp_path[MAX_LEN];
    if(path[0] != '/') { // use relative address
        strcpy(tmp_path, curr_path);
        if(!strcmp(curr_path, "/")) {
            strcat(tmp_path, "/");
        }
        strcat(tmp_path, path);
    }
    else {
        strcpy(tmp_path, path);
    }
    
    abs_path[0] = '/';
    abs_path[1] = '\0';
    int idx = 1;
    int prev_len = 1;
    char root_path[MAX_LEN];
    char *leaftname = tmp_path;
    while(1) {
        leaftname = get_rootname(root_path, leaftname);

        if(strcmp(root_path, "..")) {
            idx -= (prev_len + 1);
            if(idx < 1) {
                idx = 1;
                abs_path[0] = '/';
            }
            abs_path[idx] = '\0';
        }
        else if(strcmp(root_path, ".")) {
            // pass
        }
        else {
            strcat(abs_path + idx, root_path);
            prev_len = strlen(root_path);
            idx += prev_len;
            abs_path[idx++] = '/';
            abs_path[idx] = '\0';
        }

        if(leaftname == NULL) {
            if(abs_path[idx-1] == '/') abs_path[idx-1] = '\0';
            else abs_path[idx] = '\0';
            break;
        }
    }
}