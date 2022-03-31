#ifndef COMMAND_H
#define COMMAND_H

void command_help ();
void command_hello ();
void command_notfound ( char * );
void command_mailbox();
void command_reboot ();
void kernel_load();
void alloc_test();
void* simple_alloc(int size);
void sleep(char *duration);
void timeout_callback(char *msg);
#endif