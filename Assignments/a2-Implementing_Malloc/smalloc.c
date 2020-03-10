#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "smalloc.h"


void *mem;
typedef struct block Block;
Block *freelist;
Block *allocated_list;

/* 
 * Helper functions
 */

/* 
 * Change number that is not divisble by 8 to number that is 
 * divisible by 8. This is for "aligned on an 8-byte boundary".
 */
unsigned int filter(unsigned int nbytes) {
    unsigned int result;
    if ((nbytes % 8) != 0) {
        result = nbytes / 8;
        result ++;
        return (result * 8);
    }
    return nbytes;
}

/*
 * Traverse through linked list to see the whole list. It prints
 * every member of each block in the list. (Test purpose)
 */
void traverse(Block *curr) {
    while (curr != NULL) {
        printf("tra curr-addr %p\n", curr->addr);
        printf("tra curr-size %d\n", curr->size);
        printf("tra curr-next %p\n", curr->next);
        curr = curr->next;
    }
}

/*
 * Traverse through linked list and free every block that has
 * used malloc.
 */
void clean(Block *curr) {
    Block *temp;
    while (curr != NULL) {
        temp = curr->next;
        free(curr);
        curr = temp;
    }
}

/*
 * Traverse through freelist and search for a block whose size is 
 * bigger than or equal to given filtered unsigned int. There are 3 cases.

 * 1. If there is no block that meets this criteria, then return 0. 
 *
 * 2. If there is a block whose size is equal to filtered unsigned 
 * int, then return 1. 
 *
 * 3. If there ia a block whose size is bigger than or equal to  
 * filtered unsigned int, then return 2. 
 */
int search_size(unsigned int required) {
    int result = 0;
    Block *curr = freelist;
    while (curr != NULL) {
        if (curr->size < required) {
            curr = curr->next;
        } else if (curr->size == required) {
            result = 1;
            break;
        } else {
            result = 2;
            break;
        }
    }
    return result;
}

/*
 * Traverse through freelist and search for a address of block whose size is 
 * bigger than or equal to given filtered unsigned int. It returns array of 
 * pointers to block which includes pointer to current block(which has adequate 
 * size) and pointer to previous block.
 */
Block ***search_size_location(unsigned int required) {
    Block *curr = freelist;
    Block *prev = NULL;
    Block ***ptrs = malloc(2 * sizeof(Block **));
    ptrs[0] = malloc(sizeof(Block *));
    ptrs[1] = malloc(sizeof(Block *));
    while (curr != NULL) {
        if (curr->size < required) {
            prev = curr;
            curr = curr->next;
        } else {
            *ptrs[0] = curr;
            *ptrs[1] = prev;
            break;
        }
    }
    *ptrs[0] = curr;
    *ptrs[1] = prev;
    return ptrs;
}

/*
 * Traverse through freelist and search for a address of block whose address is 
 * bigger than or equal to given filtered unsigned int. It returns array of 
 * pointers to block which includes pointer to current block(which has adequate 
 * size) and pointer to previous block.
 */
Block ***search_addr_location(void *address) {
    Block *curr = freelist; 
    Block *prev = NULL;
    Block ***ptrs = malloc(2 * sizeof(Block **));
    ptrs[0] = malloc(sizeof(Block *));
    ptrs[1] = malloc(sizeof(Block *));
    while (curr != NULL) {
        if (curr->addr < address) {
            prev = curr;
            curr = curr->next;
        } else {
            *ptrs[0] = curr;
            *ptrs[1] = prev;
            break;
        }
    }
    *ptrs[0] = curr;
    *ptrs[1] = prev;
    return ptrs;
}

/*
 * Traverse through freelist and search for a block whose address is 
 * equal to given address. There are 3 cases.

 * 1. If there is no block that meets this criteria, then return NULL. 
 *
 * 2. If there is a block whose address is equal to given address,
 * then return array of block ** which includes pointer to current block
 * and pointer to previous block.
 *
 */
Block ***search_address(void *addr) {
    int result = 0;
    Block *curr = allocated_list;
    Block *prev = NULL;
    Block ***ptrs = malloc(2 * sizeof(Block **));
    ptrs[0] = malloc(sizeof(Block *));
    ptrs[1] = malloc(sizeof(Block *));
    while (curr != NULL) {
        if (curr->addr != addr) {
            prev = curr;
            curr = curr->next;
        } else {
            result = 1;
            *ptrs[0] = curr;
            *ptrs[1] = prev;
            break;
        }
    }
    if (result == 1) {
        return ptrs;
    }
    return NULL;
}

/*
 * Create a new struct block at heap.
 */
Block *create_block(void *addr, int size, Block *next) {
    Block *new_block = malloc(sizeof(Block));
    new_block->addr = addr;
    new_block->size = size;
    new_block->next = next;
    return new_block;
}

/*
 * Main functions
 */

/*
 * Reserves nbytes bytes of space from the memory region created by mem_init. 
 * If the memory is reserved (allocated) successfully, Returns a pointer to the 
 * reserved memory. If the memory cannot be reserved (i.e. there is no block that 
 * is large enough to hold nbytes bytes), returns NULL.
 */
void *smalloc(unsigned int nbytes) {
	//TODO
    unsigned int required = filter(nbytes);
    int result;
    result = search_size(required);
    Block *adeq_free;
    Block *prev_free;
    if (allocated_list != NULL) {
        Block ***bptrs = search_size_location(required);
        adeq_free = *bptrs[0];
        prev_free = *bptrs[1];
        free(bptrs[0]);
        free(bptrs[1]);
        free(bptrs);
    } else {
        adeq_free = freelist;
        prev_free = NULL;
    }
    if (result == 0) {
        fprintf(stderr, "Error: There is no memory with enough size\n"); 
        return NULL;
    } 
    else if (result == 2) {
        allocated_list = create_block(adeq_free->addr, required, allocated_list);
        adeq_free->addr = adeq_free->addr + required;
        adeq_free->size = adeq_free->size - required;
    } 
    else {
        if (prev_free != NULL) {
            // if (adeq_free->next == NULL) {
                
            // }
            // freelist = adeq_free->next;
            prev_free->next = adeq_free->next;
            adeq_free->next = allocated_list;
            allocated_list = adeq_free;
        } else {
            freelist = adeq_free->next;
            adeq_free->next = allocated_list;
            allocated_list = adeq_free;
        }
    }
    // printf("allocated_list: \n");
    // traverse(allocated_list);
    // printf("freelist: \n");
    // traverse(freelist);
    // printf("freelist= %p\n", freelist);
    return allocated_list->addr;
}


/*
 * Returns memory allocated by smalloc to the list of free blocks so that it 
 * might be reused later.
 */
int sfree(void *addr) {
	//TODO
    Block ***fptrs = search_addr_location(addr);
    Block *adeq_free = *fptrs[0];
    Block *prev_free = *fptrs[1];
    Block ***bptrs = search_address(addr);
    Block *adeq = *bptrs[0];
    Block *prev = *bptrs[1];
    free(fptrs[0]);
    free(fptrs[1]);
    free(fptrs);
    free(bptrs[0]);
    free(bptrs[1]);
    free(bptrs);

    if (fptrs == NULL) {
        fprintf(stderr, "Error: There is no memory with given memory address\n"); 
        return -1;
    } 
    else {
        if (freelist == NULL) {
            if (prev != NULL) {
                prev->next = adeq->next;
            } else {
                allocated_list = adeq->next;
            }
            freelist = adeq;
            adeq->next = NULL;
        } else {
            if (prev_free != NULL) {
                if (prev != NULL) {
                    prev->next = adeq->next;
                } else {
                    allocated_list = adeq->next;
                }
                prev_free->next = adeq;
                adeq->next = adeq_free;
            } else {
                if (prev != NULL) {
                    prev->next = adeq->next;
                } else {
                    allocated_list = adeq->next;
                }
                adeq->next = adeq_free;
                freelist = adeq;
            }
        }
    }
    // printf("allocated_list: \n");
    // traverse(allocated_list);
    // printf("freelist: \n");
    // traverse(freelist);
    // printf("freelist= %p\n", freelist);
    return 0;
}


/* Initialize the memory space used by smalloc,
 * freelist, and allocated_list
 * Note:  mmap is a system call that has a wide variety of uses.  In our
 * case we are using it to allocate a large region of memory. 
 * - mmap returns a pointer to the allocated memory
 * Arguments:
 * - NULL: a suggestion for where to place the memory. We will let the 
 *         system decide where to place the memory.
 * - PROT_READ | PROT_WRITE: we will use the memory for both reading
 *         and writing.
 * - MAP_PRIVATE | MAP_ANON: the memory is just for this process, and 
 *         is not associated with a file.
 * - -1: because this memory is not associated with a file, the file 
 *         descriptor argument is set to -1
 * - 0: only used if the address space is associated with a file.
 */
void mem_init(int size) {
    mem = mmap(NULL, size,  PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if(mem == MAP_FAILED) {
         perror("mmap");
         exit(1);
    }
	//TODO
    Block *new_free = malloc(sizeof(Block));
    new_free->addr = mem; 
    new_free->size = size;
    new_free->next = NULL;
    freelist = new_free;
    allocated_list = NULL;
}

/*
 * Uses helper function clean to free all the dynamically allocated memory 
 * (allocated_list and freelist) used by the program before exiting. 
 */
void mem_clean(){
    //TODO
    clean(freelist);
    clean(allocated_list);
}
