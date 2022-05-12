int strcmp(char *s1, char *s2) {
    while (*s1 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }
    return (*(unsigned char *)s1) - (*(unsigned char *)s2);
}

void strcpy(char *dest, char *src) {
    while (*src != '\0') {
        *dest = *src;
        src++;
        dest++;
    }
    *dest = *src;
}

int strlen(char *str) {
    int len = 0;
    while (*str != '\0') {
        len++;
        str++;
    }
    return len;
}