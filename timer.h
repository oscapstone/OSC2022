typedef struct timer_list{
    struct timer_list *next, *prev;
    unsigned int priority;
    char argv[100];
    void (*callback)();
} timer_list;
void cb_message(char* message);
void cb_message_delay();
void timer_init();
void add_timer(timer_list *task);
void handle_timer_list();