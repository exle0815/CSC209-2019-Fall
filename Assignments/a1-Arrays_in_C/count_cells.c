#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.c"

void read_image(int num_rows, int num_cols, 
                int arr[num_rows][num_cols], FILE *fp);
                
void print_image(int num_rows, int num_cols, int arr[num_rows][num_cols]);

// Remember to include the function prototype for count_cells
int count_cells(int num_rows, int num_cols, int arr[num_rows][num_cols]);

int main(int argc, char **argv) {
    // Print a message to stderr and exit with an argument of 1 if there are
    // not the right number of parameters, or the second argument is not -p
	if (argc == 2 || argc == 3) {
		if ((argc == 3) && (strcmp(argv[2], "-p") != 0)) {
		fprintf(stderr, "Usage: count_cells <imagefile.txt> [-p]\n");
		exit(1);
		}
	} else {
		fprintf(stderr, "Usage: count_cells <imagefile.txt> [-p]\n");
		exit(1);
	}
	int p, q, result;
	FILE *fp = NULL;
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "File open error. File does not exist.\n");
		exit(1);
	}
	fscanf(fp, "%d %d", &p, &q);
	int a[p][q];
	read_image(p, q, a, fp);
	result = count_cells(p, q, a);
	if ((argc == 3) && (strcmp(argv[2], "-p") == 0)) {
		print_image(p, q, a);
	}
	printf("Number of Cells is %d\n", result);
	fclose(fp);

    return 0;
}
