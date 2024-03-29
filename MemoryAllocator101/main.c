#include <stdio.h>
#include <memory.h>

void *malloc(size_t size)
{
    void *block;
    block = sbrk(size);
    if (block == (void*) -1)
    {
        return NULL;
    }
    return block;
}

struct header_t
{
    size_t size;
    unsigned is_free;
    struct header_t *next;
};

