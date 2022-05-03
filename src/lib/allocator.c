#include "allocator.h"

object_allocator obj_alloc_pool[ALLOCATOR_POOL_SIZE * 4];
unsigned int object_allocator_gid = 0;

dynamic_allocator dynamic_alloc_pool[ALLOCATOR_POOL_SIZE];
unsigned int dynamic_allocator_gid = 0;

void memory_init()
{
	object_allocator_gid = 0;
	dynamic_allocator_gid = 0;
}

void dynamic_test()
{
	unsigned long long addr1;
	
	uart_puts("\n================================== dynamic allocator ==================================\n");	
    buddy_init();
    dynamic_allocator *dyalloc =  dynamic_allocator_init();
	
	uart_puts("============= init end =====================\n");
	uart_puts("[Dynamic Allocator] allocate 16 bytes...\n");
    addr1 = dynamic_alloc(dyalloc, 16);
	
	uart_puts("[Dynamic Allocator] free addr...\n");
	dynamic_free(dyalloc, addr1);

}

void show_allocator_alloc_message(unsigned long long addr, int obj_size)
{
	uart_puts("[Dynamic Allocator] allocate object ");
	uart_hex(obj_size);
	uart_puts(" bytes at ");
	uart_hex(addr);
	uart_puts(" \n\n");
}

void show_allocator_free_message(unsigned long long addr, int obj_size)
{
	uart_puts("[Dynamic Allocator] free object ");
	uart_hex(obj_size);
	uart_puts(" bytes at ");
	uart_hex(addr);
	uart_puts(" \n\n");
}

void object_allocator_request_page(object_allocator *self)
{
	int page = buddy_alloc(0);
	
	int thispage_Id = self->page_count;
	self->max_pool[thispage_Id] = self->max_pool_init;
	self->cur_pool[thispage_Id] = 0;
	self->page[thispage_Id] = page;
	self->page_count ++;
}

object_allocator* object_allocator_init(int size)
{
	int thisId = object_allocator_gid;
	object_allocator_gid++;
	
	obj_alloc_pool[thisId].page_count = 0;
	obj_alloc_pool[thisId].obj_size = size;
	obj_alloc_pool[thisId].max_pool_init = PAGE_SIZE / size;

	object_allocator_request_page(&obj_alloc_pool[thisId]);

	return &obj_alloc_pool[thisId];
}

unsigned long long obj_alloc(object_allocator *self)
{
	int pool_id = self->page_count-1;	
	
	self->cur_pool[pool_id] += 1;
	if (self->cur_pool[pool_id] > self->max_pool_init)
	{
		object_allocator_request_page(self);
		unsigned long long retAddr = obj_alloc(self);
		return retAddr;
	}
	
	unsigned int offset = (self->cur_pool[pool_id] - 1) * self->obj_size;

	unsigned long long retAddr = self->page[pool_id] * PAGE_SIZE + offset;
	retAddr = MEMORY_START +retAddr; 

    show_allocator_alloc_message(retAddr, self->obj_size);

	return retAddr;
}

int obj_free(object_allocator *self, unsigned long long phyAddr)
{
	int page = (phyAddr - MEMORY_START) >> 12;
	
	int page_id = -1;
	for(int i = 0; i < self->page_count; i++)
	{
		if(self->page[i] == page)
		{
			page_id = i;
			break;
		}
	}

	if(page_id == -1) 
		return 0;

	self->cur_pool[page_id] -= 1;

    show_allocator_free_message(phyAddr, self->obj_size);
    
	return 1;
}

dynamic_allocator* dynamic_allocator_init()
{
	int thisId = dynamic_allocator_gid;
	dynamic_allocator_gid++;
	
	if (dynamic_allocator_gid > ALLOCATOR_POOL_SIZE)
	{
		return 0;
	}
	
	dynamic_alloc_pool[thisId].obj_allocator_16 = object_allocator_init(16);
	dynamic_alloc_pool[thisId].obj_allocator_32 = object_allocator_init(32);
	dynamic_alloc_pool[thisId].obj_allocator_64 = object_allocator_init(64);
	dynamic_alloc_pool[thisId].obj_allocator_128 = object_allocator_init(128);
	dynamic_alloc_pool[thisId].obj_allocator_256 = object_allocator_init(256);
	dynamic_alloc_pool[thisId].obj_allocator_512 = object_allocator_init(512);
	dynamic_alloc_pool[thisId].obj_allocator_1024 = object_allocator_init(1024);
	dynamic_alloc_pool[thisId].obj_allocator_2048 = object_allocator_init(2048);
	dynamic_alloc_pool[thisId].obj_allocator_4096 = object_allocator_init(4096);

	return &dynamic_alloc_pool[thisId];
}

unsigned long long dynamic_alloc(dynamic_allocator *self, int req_size)
{	
	// 1. if size < obj_allocate,  use fix allocate
	unsigned long long retAddr;
	if (req_size <= 16) 
	{
		retAddr = obj_alloc(self->obj_allocator_16);
	}
	else if (req_size <= 32) 
	{
		retAddr = obj_alloc(self->obj_allocator_32);
	}
	else if (req_size <= 64) 
	{
		retAddr = obj_alloc(self->obj_allocator_64);
	}
	else if (req_size <= 128) 
	{
		retAddr = obj_alloc(self->obj_allocator_128);
	}
	else if (req_size <= 256) 
	{
		retAddr = obj_alloc(self->obj_allocator_256);
	}
	else if (req_size <= 512) 
	{
		retAddr = obj_alloc(self->obj_allocator_512);
	}
	else if (req_size <= 1024) 
	{
		retAddr = obj_alloc(self->obj_allocator_1024);
	}
	else if (req_size <= 2048)
	{		
		retAddr = obj_alloc(self->obj_allocator_2048);
	}
	else if (req_size <= 4096)
	{		
		retAddr = obj_alloc(self->obj_allocator_4096);
	}
	else
	{
		int page = buddy_alloc(req_size);
		retAddr = (unsigned long long)(MEMORY_START + page * PAGE_SIZE);
	}
	
	return retAddr;
}

void dynamic_free(dynamic_allocator *self, unsigned long long addr)
{
	if(obj_free(self->obj_allocator_32, addr)) 
	{
		return;
	}
	else if(obj_free(self->obj_allocator_64, addr))
	{		
		return;
	}
	else if(obj_free(self->obj_allocator_128, addr))
	{		
		return;
	}
	else if(obj_free(self->obj_allocator_256, addr))
	{		
		return;
	}
	else if(obj_free(self->obj_allocator_512, addr))
	{		
		return;
	}
	else if(obj_free(self->obj_allocator_1024, addr))
	{		
		return;
	}
	else if (obj_free(self->obj_allocator_2048, addr))
	{		
		return;
	}
	else if (obj_free(self->obj_allocator_4096, addr))
	{		
		return;
	}
	else
	{
		unsigned long long phyAddr = addr - MEMORY_START;
		int page = phyAddr >> 12;
		buddy_free(page);
	}
}