#pragma once
#include "utils.h"
#define CORE0_TIMER_IRQ_CTRL 0x40000040


typedef void(*callback)(char *); //message callback

typedef struct timeout_event timeout_event;

#define MAX_EVENT_MSG_LEN 20

struct timeout_event
{
    callback func;
    timeout_event* next_event;
    int total_wait_time;
    int queue_time;
    int start_time;
    char args[MAX_EVENT_MSG_LEN];
};

//uint64_t curr_time;

timeout_event* head_event;

void core_timer_enable();
void core_timer_disable();

void core_timer_handler();

void timeout_event_handler();

void set_next_timer(int secs);

void show_all_events();
void update_event_time(timeout_event* start, int time);

void print_message(char* msg);
void add_timer(callback func, char* args, int duration);
