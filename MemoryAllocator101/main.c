#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef char ALIGN[16];

union header
{
    struct
    {
        size_t size;
        unsigned is_free;
        union header *next;
    } s;
    ALIGN stub;
};
typedef union header header_t;

header_t *head, *tail;
pthread_mutex_t global_malloc_lock;

header_t *get_free_block(size_t size)
{
    header_t *curr = head;
    while(curr)
    {
        if(curr->s.is_free && curr->s.size >= size)
            return curr;
        curr = curr->s.next;
    }
    return NULL;
}

void *malloc(size_t size)
{
    size_t total_size;
    void *block;
    header_t *header;
    if(!size)
        return NULL;
    pthread_mutex_lock(&global_malloc_lock);
    header = get_free_block(size);
    if(header)
    {
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header + 1);
    }
    total_size = sizeof(header_t) + size;
    block = sbrk(total_size);
    if (block == (void*) -1)
    {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    header = block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    if(!head)
        head = header;
    if(tail)
        tail->s.next = header;
    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);

    return (void*)(header + 1);
}

void *calloc(size_t num, size_t nsize)
{
    size_t size;
    void *block;
    if(!num || !nsize)
        return NULL;
    size = num * nsize;
    if(nsize != size/num) // check multiplication overflow
        return NULL;
    block = malloc(size);
    if(!block)
        return NULL;
    memset(block, 0, size); // clear the allocated memory to all zeores
    return block;
}

void *realloc(void *block, size_t size)
{
    header_t *header;
    void *ret;
    if(!block || !size)
        return malloc(size);
    header = (header_t*)block - 1;
    if(header->s.size >= size) // if block has size to accomodate requested size, do nothing
        return block;
    ret = malloc(size);
    if(ret)
    {
        memcpy(ret, block, header->s.size); // relocate contents of block to new bigger block
        free(block);
    }

    return ret;
}

void free(void *block)
{
    header_t *header, *tmp;
    void *programbreak;
    if(!block)
        return;
    pthread_mutex_lock(&global_malloc_lock);
    header = (header_t*)block - 1;

    programbreak - sbrk(0); // gives the current value of program break
    if((char*)block + header->s.size == programbreak)
    {
        if(head == tail)
            head = tail = NULL;
        else
        {
            tmp = head;
            while(tmp)
            {
                if(tmp->s.next == tail)
                {
                    tmp->s.next = NULL;
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }
        sbrk(0 - sizeof(header_t) - header->s.size); // release this amount of memory to OS
        pthread_mutex_unlock(&global_malloc_lock);
        return;
    }
    header->s.is_free = 1;
    pthread_mutex_unlock(&global_malloc_lock);
}

/* A debug function to print the entire link list */
void print_mem_list()
{
	header_t *curr = head;
	printf("head = %p, tail = %p \n", (void*)head, (void*)tail);
	while(curr) {
		printf("addr = %p, size = %zu, is_free=%u, next=%p\n",
			(void*)curr, curr->s.size, curr->s.is_free, (void*)curr->s.next);
		curr = curr->s.next;
	}
}
