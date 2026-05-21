#include <allocator.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct alloc_block {
  size_t size;
  bool free;
  struct alloc_block *next;
} alloc_block;

void *HEAP = NULL;
size_t HEAP_USED = 0;
size_t HEAP_SIZE = 0;

alloc_block *HEAP_HEAD = NULL;
alloc_block *HEAP_TAIL = NULL;

void mem_init(void *heap, size_t size) {
  HEAP = heap;
  HEAP_SIZE = size;
}

size_t mem_usage() { return HEAP_USED; }

alloc_block *mem_least_fit(size_t size) {
  alloc_block *last_cursor = HEAP_HEAD;
  alloc_block *cursor = HEAP_HEAD;

  alloc_block *least_fit = NULL;

  size_t used = 0;
  while (cursor != NULL) {
    used += sizeof(alloc_block) + cursor->size;

    if (cursor->free && cursor->size >= size) {
      if (least_fit == NULL || cursor->size < least_fit->size) {
        least_fit = cursor;
      }
    }

    last_cursor = cursor;
    cursor = cursor->next;
  }

  HEAP_USED = used;

  // This pattern is more common that it seems.
  //
  // When recently created memory is resized, rearranging the
  // tail can reduce heap fragmentation.
  if (least_fit == NULL && last_cursor->free && last_cursor->next == NULL) {
    HEAP_SIZE += (size - last_cursor->size);
    last_cursor->size = size;
    return last_cursor;
  }

  return least_fit;
}

void *mem_init_head(size_t size) {
  HEAP_HEAD = (alloc_block *)HEAP;
  *HEAP_HEAD = (alloc_block){
      .size = size,
      .free = false,
      .next = NULL,
  };

  HEAP_TAIL = HEAP_HEAD;
  HEAP_USED = size;

  return HEAP_HEAD;
}

void *mem_create(size_t size) {
  HEAP_TAIL->next = (void *)HEAP_TAIL + sizeof(alloc_block) + HEAP_TAIL->size;
  HEAP_TAIL = HEAP_TAIL->next;

  *HEAP_TAIL = (alloc_block){
      .free = false,
      .size = size,
      .next = NULL,
  };

  HEAP_USED += size + sizeof(alloc_block);

  return (void *)HEAP_TAIL + sizeof(alloc_block);
}

void *malloc(size_t size) {
  if (size % HEAP_ALIGNMENT != 0) {
    size_t offset = HEAP_ALIGNMENT - size % HEAP_ALIGNMENT;
    size += offset;
  }

  if (HEAP_HEAD == NULL) {
    if (size >= HEAP_SIZE) {
      return NULL;
    }
    return mem_init_head(size);
  }

  alloc_block *found = mem_least_fit(size);
  if (found == NULL) {
    if (HEAP_USED + size >= HEAP_SIZE) {
      return NULL;
    }
    return mem_create(size);
  } else {
    found->free = false;
    return (void *)found + sizeof(alloc_block);
  }
}

void *realloc(void *ptr, size_t size) {
  alloc_block *block = ptr - sizeof(alloc_block);
  if (block->size >= size) {
    return ptr;
  }

  // Necessary, so malloc can rearrange this block if necessary
  //
  // In case this block of memory is the TAIL of the heap, for example,
  // setting it free will allow malloc to resize it without creating new block.
  block->free = true;

  void *new_seg = malloc(size);
  if (new_seg == NULL) {
    // Only free memory if allocation succeeds. Now we set free to false.
    block->free = false;
  }
  return new_seg;
}

void free(void *ptr) {
  alloc_block *block = ptr - sizeof(alloc_block);
  block->free = true;
}
