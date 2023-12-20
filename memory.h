#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define align(x) (((((x)-1) >> 3) << 3) + 8)

#define BLOCK_SIZE 40
#define PAGESIZE 4096
#define FIRST_FIT 0
#define BEST_FIT 1

struct s_block
{
    size_t size;
    struct s_block *next;
    struct s_block *prev;
    int free;
    void *ptr;
    char data[1];
};

typedef struct s_block *t_block;

t_block get_block(void *p);
int valid_addr(void *p);
t_block find_block(t_block *last, size_t size);
t_block extend_heap(t_block last, size_t s);
void split_block(t_block b, size_t s);
t_block fusion(t_block b);
void copy_block(t_block src, t_block dst);

void *malloc(size_t size);
void free(void *p);
void *calloc(size_t number, size_t size);
void *realloc(void *p, size_t size);
void check_heap(void *data);
void malloc_control(int mode);

#endif