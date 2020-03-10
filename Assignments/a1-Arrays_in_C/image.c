#include <stdio.h>
#include <string.h>
    
/* Reads the image from the open file fp into the two-dimensional array arr 
 * num_rows and num_cols specify the dimensions of arr
 */
void read_image(int num_rows, int num_cols, 
                int arr[num_rows][num_cols], FILE *fp) {
	int pixel, i, j;
	for (i = 0; i < num_rows; i++) {
		for (j = 0; j < num_cols; j++) {
			fscanf(fp, "%d", &pixel);
			arr[i][j] = pixel;
		}
	}
}

/* Print to standard output the contents of the array arr */
void print_image(int num_rows, int num_cols, int arr[num_rows][num_cols]) {
	int i,j;
	for (i = 0; i < num_rows; i++) {
		for (j = 0; j < num_cols; j++) {
			printf("%d ", arr[i][j]);
		} printf("\n");
	}
}
int check_cells(int i, int j, int pixel, int num_rows, int num_cols,
				int arr[num_rows][num_cols]) {
	if (arr[i][j] != pixel || i >= num_rows || i < 0 || j < 0 || j >= num_cols) {
		return 0;
	} else {
		arr[i][j] = -1;
		return 1 + check_cells(i - 1, j, pixel, num_rows, num_cols, arr) + 
		check_cells(i + 1, j, pixel,  num_rows, num_cols, arr) + 
		check_cells(i, j - 1, pixel,  num_rows, num_cols, arr) + 
		check_cells(i, j + 1, pixel,  num_rows, num_cols, arr);
	}
}

/* TODO: Write the count_cells function */

int count_cells(int num_rows, int num_cols, int arr[num_rows][num_cols]) {
	int i, j, count, pixel, result;
	count = 0;
	for (i = 0; i < num_rows; i++) {
		for (j = 0; j < num_cols; j++) {
			pixel = arr[i][j];
			if (arr[i][j] != 0 && arr[i][j] != -1) {
				result = check_cells(i, j, pixel, num_rows, num_cols, arr);
				if (result > 0) {
					count++;
				}
			}
		}
	}
	return count;
}
