#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "smalloc.h"


#define SIZE 1000


/* Test for smalloc and sfree. */

int main(void) {

    printf("Tests for smalloc.c. Descriptions are in file.\n");


    mem_init(SIZE);
    
    int i;


    /* 
     * Test 1: Call smalloc bigger bytes than mem has.
     */

    printf("Test 1: Call smalloc(1020) where size of mem is 1000\n" );

    char *test1[5]; 

    printf("Call smalloc(920)\n");
    
    test1[0] = smalloc(920);

    printf("Call smalloc(1020)\n");
    
    test1[1] = smalloc(1020);
    write_to_mem(1, test1[0], 07);
    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();

    mem_clean();

    /* 
     * Test 2: Call smalloc 3 times and sfree 1. Then call smalloc with
     * same size as sfreed one. The purpose of this test is to check that 
     * smalloc chooses an appropriate block.
     */  
    printf("Test 2: \n");

    mem_init(SIZE);

    char *test2[10];
    for(i = 0; i < 3; i++) {
        int num_bytes = (i+1) * 10;
        test2[i] = smalloc(num_bytes);
        write_to_mem(num_bytes, test2[i], i);
    }

    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();
    
    printf("freeing %p result = %d\n", test2[1], sfree(test2[1]));
    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();

    int test2_bytes = 20;
    test2[4] = smalloc(test2_bytes);
    write_to_mem(test2_bytes, test2[4], 4);

    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();

    mem_clean();


    /*
     * Test 3: Call smalloc 6 times and sfree every block until there is no
     * allocated block. The purpose of this test is to see that freelist 
     * maintains the increasing address order.
     */
    printf("Test 3: \n");

    mem_init(SIZE);

    char *test3[10];

    for(i = 0; i < 6; i++) {
        int num_bytes = (i+1) * 10;
        test3[i] = smalloc(num_bytes);
        write_to_mem(num_bytes, test3[i], i);
    }

    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();
    
    for(i = 0; i < 6; i++) {
        printf("freeing %p result = %d\n", test3[i], sfree(test3[i]));
    }

    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();

    mem_clean();


    /*
     * Test 4: Call smalloc(200) 5 times and sfree every allocated block.
     * The purpose of this test is to see that there is no error after
     * using all memories and sfree back. 
     */
     
    printf("Test 4: \n");

    mem_init(SIZE);

    char *test4[10];

    for(i = 0; i < 5; i++) {
        test4[i] = smalloc(200);
        write_to_mem(1, test4[i], i);
    }

    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();
    
    for(i = 0; i < 5; i++) {
        printf("freeing %p result = %d\n", test4[i], sfree(test4[i]));
    }

    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();

    /*
     * Test 5: Continued from test 4. Testing what will happen after all
     * memories are scattered by several blocks by calling smalloc(300).
     */

    printf("Test 5:\n");
    printf("Call smalloc(300)\n");
    test4[5] = smalloc(300);

    printf("List of allocated blocks:\n");
    print_allocated();
    printf("List of free blocks:\n");
    print_free();
    printf("Contents of allocated memory:\n");
    print_mem();

    mem_clean();

    return 0;
}
