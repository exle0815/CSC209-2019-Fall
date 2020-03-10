#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_ARGS 32
#define MAX_NAME 128

/* Create an array of arguments suitable for passing into execvp 
   from "line". line is expected to start with a tab and contain a
   action from a Makefile. Remember to ensure that the last element
   of the array is NULL.

   It is fine to use MAX_ARGS to allocate enough space for the arguments
   rather than iterating through line twice. You may want to use strtok to
   split the line into separate tokens.

   Return NULL if there are only spaces and/or tabs in the line. No memory
   should be allocated and the return value will be NULL.
 */
extern int is_comment_or_empty(char *line);

char **build_args(char *line) {
    
    // TODO    
    char **result = malloc(MAX_ARGS * sizeof(char *));
    const char delim[2] = " ";
    result[0] = strtok(line, delim);
    int i = 1;
    while ((result[i] = strtok(NULL, delim)) != NULL) {
        i++;
    } 
    result[i] = NULL;
    return result;    
}

/* Return 1 if the line contains only spaces or a comment (#)
   Return 0 if the line contains any other character before a #

   We want to ignore empty lines or lines that contain only a comment.  This
   helper function identifies such lines.
 */
int is_comment_or_empty(char *line) {
    
    for(int i = 0; i < strlen(line); i++){
        if(line[i] == '#') {
            return 1;
        }
        if(line[i] != '\t' && line[i] != ' ') {
            return 0;
        }
    }
    return 1;
}

/* Convert an array of args to a single space-separated string in buffer.
   Returns buffer.  Note that memory for args and buffer should be allocted
   by the caller.
 */
char *args_to_string(char **args, char *buffer, int size) {
    buffer[0] = '\0';
    int i = 0;
    while(args[i] != NULL) {
        strncat(buffer, args[i], size - strlen(buffer));
        strncat(buffer, " ", size - strlen(buffer));
        i++;
    }
    return buffer;
}
