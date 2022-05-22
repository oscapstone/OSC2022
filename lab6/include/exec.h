#ifndef EXEC_H
#define EXEC_H

int loadExecutable(const char *fname, unsigned long va_exe, unsigned long va_stack);
int syscall_exec(const char *name, char *const argv[]);

#endif
