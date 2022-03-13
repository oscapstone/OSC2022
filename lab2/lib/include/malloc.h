typedef unsigned long size_t;

typedef struct malloc_header malloc_header;

struct malloc_header {
  unsigned int previous;
  unsigned int chunk;
};

void* simple_malloc(size_t size);
