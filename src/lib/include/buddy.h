#ifndef __BUDDY__H__
#define __BUDDY__H__
#include "string.h"
#include "stddef.h"
#include "uart.h"
#include "utils.h"

#define MEMORY_START			0x10000000	
#define MEMORY_SIZE				0x10000000	// from TA, 0x10000000-0x20000000 
#define PAGE_SIZE				0x1000		// 4096
#define PAGE_NUM				(MEMORY_SIZE / PAGE_SIZE) // all page frame = 65536
#define BUDDY_MAX_ORDER 		16			// 2^0 ~ 2^16
#define BUDDY_ALLOCATED_NUM 	65536
#define BUDDY_NODE_LIST_NUM 	16

typedef enum
{
	FREE,
	USED
} Status;

typedef struct 
{
	int start;
	int end;
	Status status;
} buddy_node;

typedef struct 
{
	int count;
	buddy_node node_list[BUDDY_NODE_LIST_NUM];
} buddy_head;

typedef struct 
{
	int count;
	buddy_node node_list[BUDDY_ALLOCATED_NUM];
} buddy_allocated;


void buddy_init();
void show_alloc_message(buddy_node node);
void show_free_message(buddy_node node);
bool allocated_list_push(buddy_node node);
buddy_node allocated_list_pop(int node_start);
buddy_node buddy_list_pop(buddy_head *list);
void buddy_list_push(buddy_head *list, buddy_node node);
void buddy_merge(int order, buddy_node* node);
int buddy_alloc(int size);
void buddy_free(int node_start);
void buddy_test();
// void print_buddy_info();

#endif