#ifndef SHELL_H
#define SHELL_H

void shell();
void do_cmd(char* cmd);
void print_system_messages();
int  ls(char* working_dir);
int  cat(char* thefilepath);

#endif