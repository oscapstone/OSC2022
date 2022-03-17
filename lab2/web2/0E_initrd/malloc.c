#include <stdint.h>
#include "malloc.h"
void * simple_malloc(uint32_t delta)
{
    extern char _end;     /* Defined by the linker */
    extern char __heap_end;
    static char *heap_end;
    static char *heap_limit;
    char *prev_heap_end;

    if(heap_end == 0)
    {
        heap_end = &_end;
        heap_limit = &__heap_end;
    }

    prev_heap_end = heap_end;
    if(prev_heap_end + delta > heap_limit)
    {
        return((void *) -1L);
    }
    heap_end += delta;
    return((void *) prev_heap_end);
}
