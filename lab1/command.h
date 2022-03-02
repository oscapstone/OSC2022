// void input_buffer_overflow_message(char[]);

int strcmp(const char* s1, const char* s2);
void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();
void exec_reboot();

void exec_help();
void exec_hello();

void command_not_found(char*);

void mbox_board_revision();
void mbox_arm_memory();

void parse_command(char*);