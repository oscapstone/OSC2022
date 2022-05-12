#ifndef STRING_H
#define STRING_H

#define ENDL "\r\n"

void memcpy(char *s1, const char *s2, unsigned int len) {
    for (unsigned int i = 0; i < len; i++)
        s1[i] = s2[i];
}

void memset(char *s1, const char c, unsigned int len) {
    for (unsigned int i = 0; i < len; i++)
        s1[i] = c;
}

unsigned int strlen(char *s) {
    unsigned int len = 0;
    while (1) {
        if (*(s + len) == '\0')
            break;
        len++;
    }
    return len;
}

int strcmp(const char *s1, const char *s2) {
    unsigned int i = 0;
    for (i = 0; i < strlen(s1); i++) {
        if (s1[i] != s2[i])
            return s1[i] - s2[i];
    }
    return s1[i] - s2[i];
}

int strncmp(const char *s1, const char *s2, unsigned int n) {
    unsigned int i = 0;
    for (i = 0; i < n; i++) {
        if (s1[i] != s2[i])
            return s1[i] - s2[i];
    }
    return 0;
}

void strrev(char *s) {
    unsigned int len = strlen(s);
    char tmp;
    for(unsigned int i = 0; i < len/2; i++){
        tmp = s[i];
        s[i] = s[len-i-1];
        s[len-i-1] = tmp;
    }
}

// string to int
int atoi(char* s) {
    int res = 0;
    for (int i = 0; s[i] != '\0'; ++i) {
        if(s[i] > '9' || s[i] < '0')
            return res;
        res = res * 10 + s[i] - '0';
    }
    return res;
}

// int to string
char* itoa(int x) {
    char str[MAX_BUF_SIZE];
    memset(str, 0, MAX_BUF_SIZE);
    int i = 0, negative = 0;
    // handle 0 explicitly
    if (x == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
    // handle negative numbers
    if (x < 0) {
        negative = 1;
        x = -x;
    }
    // process individual digits
    while (x != 0) {
        str[i++] = x % 10 + '0';
        x = x / 10;
    }
    if (negative)
        str[i++] = '-';
    str[i] = '\0';
    strrev(str);
    return str;
}

// unsigned int to hex string
void uint_to_hex(unsigned int x, char *s) {
    int i = 0, rem = 0, len = strlen(s);
    memset(s, 0, len);
    // handle 0 explicitly
    if (x == 0) {
        s[i++] = '0';
        s[i] = '\0';
        return ;
    }
    // process individual digits
    while (x != 0) {
        rem = x % 16;
        if (rem >= 10)
            s[i++] = rem - 10 + 'A';
        else
            s[i++] = rem + '0';
        x = x / 16;
    }
    s[i] = '\0';
    strrev(s);
}

// hex string to unsigned int
unsigned int hex_to_uint(const char *s, unsigned int size) {
    unsigned int ret = 0;
    for (unsigned int i = 0; i < size; i++) {
        ret *= 16;
        if (s[i] >= '0' && s[i] <= '9') {
            ret += s[i] - '0';
        } else if (s[i] >= 'a' && s[i] <= 'f') {
            ret += s[i] - 'a' + 10;
        } else if (s[i] >= 'A' && s[i] <= 'F') {
            ret += s[i] - 'A' + 10;
        }
    }
    return ret;
}

// big endian to unsigned int
unsigned int BE_to_uint(void *ptr) {
    unsigned char *bytes = (unsigned char *)ptr;
    return bytes[3] | bytes[2] << 8 | bytes[1] << 16 | bytes[0] << 24;
}

#endif