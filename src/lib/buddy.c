#include "buddy.h"

buddy_head buddy_list[BUDDY_MAX_ORDER + 1];
buddy_allocated buddy_allocated_list;

void buddy_test()
{
	buddy_init();

    int addr1, addr2;

    uart_puts("================================== buddy system ==================================\n");
	uart_puts("start allocate addr1...\n");
    addr1 = buddy_alloc(32 * PAGE_SIZE);

	uart_puts("start free addr1...\n");
	buddy_free(addr1);

	uart_puts("start allocate addr2...\n");
    addr2 = buddy_alloc(7 * PAGE_SIZE);

	uart_puts("start free addr2...\n");
	buddy_free(addr2);

	uart_puts("====================================\n");
}

void show_alloc_message(buddy_node node)
{

	uart_puts("[Buddy system] allocate memory page ( from ");
	uart_hex(node.start);
	uart_puts(" to ");
	uart_hex(node.end);
	uart_puts(" )\n");
	uart_puts("[Buddy system] allocate memory address ( from ");
	uart_hex(node.start * PAGE_SIZE + MEMORY_START);
	uart_puts(" to ");
	uart_hex((node.end + 1) * PAGE_SIZE + MEMORY_START);
	uart_puts(" )\n\n");
	
}

void show_free_message(buddy_node node)
{

	uart_puts("[Buddy system] free memory page ( from ");
	uart_hex(node.start);
	uart_puts(" to ");
	uart_hex(node.end);
	uart_puts(" )\n");
}

bool allocated_list_push(buddy_node node)
{
	if(buddy_allocated_list.count == BUDDY_ALLOCATED_NUM)
	{
		return false;
	}

	// put node to allocated_list and show message
	node.status = USED;
	for(int i = 0; i < BUDDY_ALLOCATED_NUM; i++)
	{
		if(buddy_allocated_list.node_list[i].status == FREE) 
		{
			buddy_allocated_list.node_list[i].start = node.start;
			buddy_allocated_list.node_list[i].end = node.end;
			buddy_allocated_list.node_list[i].status = node.status;

			show_alloc_message(node);

			break;
		}
	}

	buddy_allocated_list.count++;
	return true;
}

buddy_node allocated_list_pop(int node_start)
{
	// search and pop node
	buddy_node ret_node;
	for(int i = 0; i < BUDDY_ALLOCATED_NUM; i++)
	{
		buddy_node *node = &buddy_allocated_list.node_list[i];
		if(node->status == USED && node->start == node_start) 
		{
			node->status = FREE;
			ret_node.start = buddy_allocated_list.node_list[i].start;
			ret_node.end = buddy_allocated_list.node_list[i].end;
			ret_node.status = buddy_allocated_list.node_list[i].status;

			break;
		}
	}

	buddy_allocated_list.count--;
	return ret_node;
}

// pop first buddy_node from buddy list
buddy_node buddy_list_pop(buddy_head *list)
{
	// 1. get first node
	buddy_node ret_node;
	ret_node.start = list->node_list[0].start;
	ret_node.end = list->node_list[0].end;
	ret_node.status = list->node_list[0].status;
	list->count -= 1;

	// 2. move other list node forward iteration
	int count = list->count;
	int cur = 1;
	while(count != 0)
	{
		list->node_list[cur - 1].start = list->node_list[cur].start;
		list->node_list[cur - 1].end = list->node_list[cur].end;
		list->node_list[cur - 1].status = list->node_list[cur].status;
		
		count--;
		cur++;
	}
	
	// 3. init last node
	list->node_list[cur - 1].start = list->node_list[cur - 1].end = 0;
	list->node_list[cur - 1].status = FREE;

	return ret_node;
}

// split node to two part
void buddy_list_push(buddy_head *list, buddy_node node)
{
	int size = (node.end - node.start + 1) / 2;
	
	// first half
	list->node_list[list->count].start = node.start;
	list->node_list[list->count].end = node.start + size - 1;
	list->count += 1;

	// second half
	list->node_list[list->count].start = node.start + size;
	list->node_list[list->count].end = node.end;
	list->count += 1;

	// print first half log
	uart_puts("first half (from ");
	uart_hex(node.start);
	uart_puts(" to ");
	uart_hex(node.start + size - 1);
	uart_puts(" )\n");
	// print second half log
	uart_puts("second half (from ");
	uart_hex(node.start + size);
	uart_puts(" to ");
	uart_hex(node.end);
	uart_puts(" )\n");
	
}

void buddy_merge(int order, buddy_node* node)
{
	buddy_node merge_node;
	bool can_merge = false;
	int merge_index;
	int size = buddy_list[order].count;
	
	//char buf[16] = {0};

	while(1)
	{
		can_merge = false;
		size = buddy_list[order].count;

		// calc free node and list node can merge (continue memory)
		for(merge_index = 0; merge_index < size; merge_index++)
		{
			if(buddy_list[order].node_list[merge_index].end + 1 == node->start)
			{
				merge_node.start = buddy_list[order].node_list[merge_index].start;
				merge_node.end = node->end;
				can_merge = true;
				break;
			}

			if(buddy_list[order].node_list[merge_index].start == node->end + 1)
			{
				merge_node.start = node->start;
				merge_node.end = buddy_list[order].node_list[merge_index].end;
				can_merge = true;
				break;
			}		
		}

		// if can merge, show message, and merge next level iteratively
		if(can_merge)
		{

			// uart_puts("[Buddy system] merge ( from ");
			// uart_hex(buddy_list[order].node_list[size - 1].start);
			// uart_puts(" to ");
			// uart_hex(buddy_list[order].node_list[size - 1].end);
			// uart_puts(" ) and ( from ");
			// uart_hex(node->start);
			// uart_puts(" to ");
			// uart_hex(node->end);
			// uart_puts(" )\n");


			if(size - 1 == 0)
			{
				buddy_list[order].count = 0;
			}
			else
			{
				buddy_list[order].node_list[merge_index].start = buddy_list[order].node_list[size].start;
				buddy_list[order].node_list[merge_index].end = buddy_list[order].node_list[size].end;
				buddy_list[order].node_list[merge_index].status = buddy_list[order].node_list[size].status;
				buddy_list[order].count-- ;
			}

			if(order != BUDDY_MAX_ORDER)	
			{			
				//buddy_merge(order + 1, &merge_node);
				node->start = merge_node.start;
				node->end = merge_node.end;
				order = order + 1;
			}
		}
		else
		{
			// if can't merge, free node add to list
			buddy_list[order].node_list[size].start = node->start;
			buddy_list[order].node_list[size].end = node->end;
			buddy_list[order].node_list[size].status = node->status;
			buddy_list[order].count++;
			break;
		}
	}
}

void buddy_init()
{
	// init buddy_list
	for(int i = 0; i < BUDDY_MAX_ORDER + 1; i++)
		buddy_list[i].count = 0;

	// only the last buddy has free page 0-65535
	buddy_list[BUDDY_MAX_ORDER].count = 1;
	buddy_list[BUDDY_MAX_ORDER].node_list[0].start = 0;
	buddy_list[BUDDY_MAX_ORDER].node_list[0].end = PAGE_NUM - 1;
	buddy_list[BUDDY_MAX_ORDER].node_list[0].status = FREE;	

	// init buddy_allocated_list
	buddy_allocated_list.count = 0;
	for(int i = 0; i < BUDDY_ALLOCATED_NUM; i++)
		(buddy_allocated_list.node_list[i]).status = FREE;

}

int buddy_alloc(int size)
{	
	// 1. calc buddy order for allocate size 
	// ex. size = 8192, order = 1
	int order = 0;
	while(1)
	{
		if (size <= PAGE_SIZE * (1 << order))
			break;
		
		order++;
	}
	if (order > BUDDY_MAX_ORDER)
	{
		return -1;
	}

	// 2. if has free buddy, allocate and return
	if(buddy_list[order].count > 0)
	{
		buddy_node alloc_node = buddy_list_pop(&buddy_list[order]);
		if(allocated_list_push(alloc_node))
			return alloc_node.start;
		else
			return -1;
	}

	// 3. if not, search high level buddy and spilt buddy / 2
	int new_order = order + 1;
	while(order < BUDDY_MAX_ORDER + 1)
	{
		if(buddy_list[new_order].count > 0) 
			break;
		
		new_order++;
	}
	if(new_order == BUDDY_MAX_ORDER + 1)
	{
		return -1;
	}
	
	buddy_node temp_node = buddy_list_pop(&buddy_list[new_order]);
	new_order--;

	// 3.1 spilt up until allocate size
	while(new_order >= order)
	{
		// split node to two part:
		
		buddy_list_push(&buddy_list[new_order], temp_node);
		
		// pop first part to next level
		temp_node = buddy_list_pop(&buddy_list[new_order]);
		
		new_order--;
	}

	// 4. save to allocated_list and return address
	if (allocated_list_push(temp_node))
		return temp_node.start;
	else
		return -1;
}

void buddy_free(int node_start)
{
	buddy_node tmpNode = allocated_list_pop(node_start);
	show_free_message(tmpNode);
	
	int size = tmpNode.end - tmpNode.start + 1;
	int order = log2(size);
	
	buddy_merge(order, &tmpNode);
}