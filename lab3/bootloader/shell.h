#ifndef SHELL_H
#define SHELL_H

#define MAX_BUFFER_LEN 128

enum CHAR_CLASS
{
    BACKSPACE = 8,
    LINE_FEED = 10,
    CARRIAGE_RETURN = 13,
    
    
    NORMAL_CHAR = 1000,
    NEW_LINE = 1001,
    
    UNKNOWN = -1,

};

void shell_start();
enum CHAR_CLASS analysis(char input_char);
void command_proccess(char input_char, char *buffer, int *buffer_counter);

#endif
