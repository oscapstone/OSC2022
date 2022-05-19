#ifndef EXEC_H
#define EXEC_H

void* loadExecutable(const char *fname);
int syscall_exec(const char *name, char *const argv[]);

#endif
