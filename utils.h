typedef struct List {
    struct List *next;
    void *entry;
} List;

void delay(int t);
int strcmp(const char* s1, const char* s2);
int strlen(const char *str);
int strncmp(char* s1, char* s2);
void strcpy(char *dest, char *src);
void strncpy(char *dest, char *src, unsigned int size);
unsigned int sprintf(char *dst, char* fmt, ...);
unsigned int vsprintf(char *dst,char* fmt, __builtin_va_list args);
char *scanf(char *fmt, ...);
unsigned int c_toi(char* str);