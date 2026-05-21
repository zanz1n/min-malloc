# min-malloc

A stupid simple memory allocator.

# How does it work?

Basically, you give it a pointer to a memory segment that will be used as the heap. Before each
allocation, the allocator fills the 24 bytes before the memory segment with metadata, which includes
size, if the segment is free or not, and a pointer to the next block. After that, the metadata of
the previously allocated block will be modified, so its next-segment field (previously NULL) point
to this new segment.

All allocations are aligned as 128 bytes, to avoid fragmentations caused by a large number of small
objects.

The aligment can be changed by the `HEAP_ALIGNMENT` macro (128 by default).

## `free()`

When free is called to a pointer, the allocator looks at the 24 bytes of metadata metadata before
the start of the pointer and marks it as free, so it can be reused.

## `malloc()`

Malloc does not expand the HEAP imediatelly. First, it looks for the blocks marked as free and tries
to find the segment of minimal size that fits this new data. If none can be found, it inserts a new
one at the end, and sets the TAIL->next field to this new block.

## `realloc()`

First, it inspects the metadata of the block to check if it fits the new size. When it doesn't, the
block is marked as free and `malloc()` is called. If it fails, the free flag of the segment is set
to false and NULL is returned.

It does things in such way, so the correct implementation of realloc is preserved, in which realloc
only gives the original memory back to the allocator if the new allocation was successfull.
Furthermore, marking the segment as free before calling malloc allows the allocator to resize the
block if needed, without touching its data.
