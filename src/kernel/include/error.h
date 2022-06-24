#ifndef _DEF_ERROR
#define _DEF_ERROR

void kmsg_(const char *file, int line, const char *function, char *message, ...);
void kpanic_(const char *file, int line, const char *function, char *message, ...);

#define kmsg(...) kmsg_(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define kpanic(...) kpanic_(__FILE__, __LINE__, __func__, __VA_ARGS__)

#endif