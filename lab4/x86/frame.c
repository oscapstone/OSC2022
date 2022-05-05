#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MEMORY_BASE 0x0
#define MEMORY_LIMIT 0x3C000000
#define FRAME_SIZE 0x1000

#define FREE_BODY -1
#define ALLOCATED -3

#define MAX_EXP 5
// 2^0 ~ 2^5

#define FRAME_IDX(frame_list_node) (frame_list_node-frame_list)
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

struct FrameList {
  struct FrameList *next;
  struct FrameList *prev;
};

struct FrameList frame_list[MEMORY_LIMIT / FRAME_SIZE];

int frame_array[MEMORY_LIMIT / FRAME_SIZE];
struct FrameList *free_list[MAX_EXP + 1];
struct FrameList *last_elm[MAX_EXP + 1];

void append_frame(struct FrameList *fp, int exp) {
  if (free_list[exp] == NULL) {
    free_list[exp] = fp;
    last_elm[exp] = fp;
    fp->prev = NULL;
    fp->next = NULL;
  } else {
    fp->prev = last_elm[exp];
    fp->next = NULL;
    last_elm[exp]->next = fp;
    last_elm[exp] = fp;
  }
}

void remove_frame(struct FrameList *fp, int exp) {
  
  if (fp->prev != NULL) {
    fp->prev->next = fp->next;
  }

  if (fp->next != NULL) {
    fp->next->prev = fp->prev;
  }


  if (free_list[exp] == fp) {
    free_list[exp] = fp->next;
  }
  if (last_elm[exp] == fp) {
    last_elm[exp] = last_elm[exp]->prev;
  }
  
  fp->prev = NULL;
  fp->next = NULL;
}


void init_frame_allocator() {
  const int conti_size = (1<<MAX_EXP);
  int i = 0;
  memset(frame_array, -1, sizeof(frame_array));
  for (i = 0; i < (MEMORY_LIMIT / FRAME_SIZE); i += conti_size) {
    frame_array[i] = MAX_EXP;
    append_frame(&frame_list[i], MAX_EXP);
  }
}

/* this function return the index of the allocated frame */
int allocate_frame(int reqexp) {
  if (reqexp > MAX_EXP || reqexp < 0) {
    return -1;
  }

  if (free_list[reqexp] == NULL) {
    // request from a larger block
    int upper_exp = -1;
    for (int i = reqexp+1; i <= MAX_EXP; i++) {
      if (free_list[i] != NULL) {
        upper_exp = i;
        break;
      }
    }
    
    if (upper_exp < 0) {
      return -1;
    }

    while (upper_exp > reqexp) {
      struct FrameList *ff = free_list[upper_exp];
      remove_frame(ff, upper_exp);
      upper_exp--;
      struct FrameList *ff2 = ff + (1<<upper_exp);
      frame_array[FRAME_IDX(ff)] = upper_exp;
      frame_array[FRAME_IDX(ff2)] = upper_exp;
      append_frame(ff, upper_exp);
      append_frame(ff2, upper_exp);
    }
  }
  
  struct FrameList* free_frame = free_list[reqexp];
  remove_frame(free_frame, reqexp);
  int idx = FRAME_IDX(free_frame);

  for (int i = 0; i < (1<<reqexp); i++) {
    frame_array[idx+i] = ALLOCATED;
  }
  
  return idx;
}


int deallocate_frame(int idx) {
  if (frame_array[idx] != ALLOCATED) return -1;
  frame_array[idx] = 0;
  append_frame(&frame_list[idx], 0);
  // merge free block with body
  for (int i = 0; i < MAX_EXP; i++) {
    int buddy_idx = idx ^ (1<<i);
    if (frame_array[buddy_idx] == i) {
      printf("merge %d %d -> 2^%d\n", idx, buddy_idx, i+1);
      remove_frame(&frame_list[buddy_idx], i);
      remove_frame(&frame_list[idx], i);
      if (buddy_idx < idx) {
        int tmp = idx;
        idx = buddy_idx;
        buddy_idx = tmp;
      }
      frame_array[idx] = i+1;
      frame_array[buddy_idx] = FREE_BODY;
      append_frame(&frame_list[idx], i+1);
    } else {
      break;
    }
  }
  return 0;
}

int deallocate_cont_frame(int idx, int num) {
  for (int i = 0; i < num; i++)
    deallocate_frame(idx+i);
  return 0;
}

void traverse_list(struct FrameList* frame) {
  printf("traverse %ld", FRAME_IDX(frame));
  int num = 0;
  frame = frame->next;
  while (frame != NULL) {
    num++;
    if (num >= 5) {
      printf("...");
      break;
    }
    printf(" -> %ld", FRAME_IDX(frame));
    frame = frame->next;
  }
  putchar('\n');
}

void traverse_order() {
  for (int i = 0; i <= MAX_EXP; i++) {
    printf("2^%d: ", i);
    if (free_list[i] != NULL)
      traverse_list(free_list[i]);
    else
      printf("(null)\n");
  }
  printf("\n");
}


int main() {
  init_frame_allocator();

  printf("frame num %d\n", MEMORY_LIMIT / FRAME_SIZE);
  
  int idx1 = allocate_frame(3);
  traverse_order();
  int idx2 = allocate_frame(1);
  traverse_order();
  int idx3 = allocate_frame(4);
  traverse_order();
  int idx4 = allocate_frame(0);
  traverse_order();
  int idx5 = allocate_frame(0);
  traverse_order();

  printf("%d %d %d %d %d\n", idx1, idx2, idx3, idx4, idx5);

  deallocate_cont_frame(idx1, (1<<3));
  traverse_order();
  deallocate_cont_frame(idx2, (1<<1));
  traverse_order();
  deallocate_cont_frame(idx3, (1<<4));
  traverse_order();
  deallocate_cont_frame(idx4, (1<<0));
  traverse_order();
  deallocate_cont_frame(idx5, (1<<0));
  traverse_order();

  printf("deallocated successfully\n");
  
  return 0;
  
}
