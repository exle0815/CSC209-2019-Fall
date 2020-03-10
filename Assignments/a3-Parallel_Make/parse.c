#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include "pmake.h"

/*
 * Count size of the string array.
 */
int count_size(char **list) {
    int i = 0;
    while (list[i] != NULL) {
        i++;
    }
    return i;
}

/*
 * Trim the string.
 */
void trim(char *str, int size) {
    char new[size];
    int i;
    int j = 0;
    for (i = 0; i < size; i++) {
        if (isalpha(str[i]) != 0 || str[i] == '.' || isdigit(str[i]) != 0 || str[i] == '-') {
            new[j] = str[i];
            j++;
        }
    }
    new[j] = '\0';
    strncpy(str, new, size);
}

/*
 * Search the rule whose target is string target. Start from the first rule.
 * If there is no rule with appropriate target, return NULL.
 */
Rule *search_rule(Rule *first, char *target) {
    Rule *curr = first;
    while (curr != NULL) {
        if (strcmp(curr->target, target) != 0) {
            curr = curr->next_rule;
        } else {
            break;
        }
    }
    return curr;
}

/*
 * Create new rule whose target is string target and other members are all NULL.
 */
Rule *create_rule(char *target) {
    Rule *new_rule = malloc(sizeof(Rule));
    // char *str = malloc(sizeof(char) * (strlen(target) + 1));    
    char *str = malloc(sizeof(char) * (strlen(target) + 1));    
    // strcpy(str, target);
    for (int i = 0; i < strlen(target); i++) {
        str[i] = target[i];
    }
    str[strlen(target)] = '\0';
    new_rule->target = str;
    new_rule->dependencies = NULL;
    new_rule->actions = NULL;
    new_rule->next_rule = NULL;
    return new_rule;
}

/*
 * Copy the args to new string array and create action whose args is 
 * copied string array.
 */
Action *create_action(char **args, int size) {
    Action *new_action = malloc(sizeof(Action));
    int i;
    char **str_array = malloc((size + 1) * sizeof(char *));
    for (i = 0; i < size; i++) {
        str_array[i] = malloc(sizeof(char) * (strlen(args[i]) + 1));
        strcpy(str_array[i], args[i]);        
    }
    str_array[size] = NULL;
    new_action->args = str_array;
    new_action->next_act = NULL;
    return new_action;
}

/*
 * Create new dependency whose members are all NULL.
 */
Dependency *create_dependency() {
    Dependency *new_dependency = malloc(sizeof(Dependency));
    new_dependency->rule = NULL;
    new_dependency->next_dep = NULL;
    return new_dependency;
}

/* Print the list of actions */
void print_actions(Action *act) {
    while(act != NULL) {
        if(act->args == NULL) {
            fprintf(stderr, "ERROR: action with NULL args\n");
            act = act->next_act;
            continue;
        }
        printf("\t");

        int i = 0;
        while(act->args[i] != NULL) {
            printf("%s ", act->args[i]) ;
            i++;
        }
        printf("\n");
        act = act->next_act;
    }    
}

/* Print the list of rules to stdout in makefile format. If the output
   of print_rules is saved to a file, it should be possible to use it to 
   run make correctly.
 */
void print_rules(Rule *rules){
    Rule *cur = rules;
    
    while(cur != NULL) {
        if(cur->dependencies || cur->actions) {
            // Print target
            printf("%s : ", cur->target);
            
            // Print dependencies
            Dependency *dep = cur->dependencies;
            while(dep != NULL){
                if(dep->rule->target == NULL) {
                    fprintf(stderr, "ERROR: dependency with NULL rule\n");
                }
                printf("%s ", dep->rule->target);
                dep = dep->next_dep;
            }
            printf("\n");
            
            // Print actions
            print_actions(cur->actions);
        }
        cur = cur->next_rule;
    }
}

/*
 * If previous line was target, put current acition to target if previous line,
 * was action, put current action next to previous action.
 */
void new_action(Rule *prev_rule, Action *new_act, Action *prev_act, int prev) {
    if (prev == 1) {   
        prev_act->next_act = new_act;             
    } else {
        prev_rule->actions = new_act;
    }   
}

/*
 * Check rule. When there is no rule yet, create new rule and make it a first rule.
 * If target is not dependency of other targets, create new rule and put it next to
 * last rule. 
 * If target is dependency of other targets, search the rule and make it to be a
 * current rule. 
 */
Rule *check_rule(char *str, Rule **first_rule, Rule *child_rule) { 
    Rule *cur;     
    if (*first_rule == NULL) {
        trim(str, strlen(str) + 1);
        *first_rule = create_rule(str);                            
        cur = *first_rule;                                                            
    } else if (search_rule(*first_rule, str) == NULL) { 
        trim(str, strlen(str) + 1);
        Rule *new_rule = create_rule(str);    
        child_rule->next_rule = new_rule;      
        cur = new_rule;               
    } else {
        cur = search_rule(*first_rule, str);                                    
    } 
    return cur;                        
}

/*
 * Clean the allocated data by traversing through rules.
 */
void clean(Rule *curr) {
    Rule *temp;
    while (curr != NULL) {
        temp = curr->next_rule;
        int i = 0;                    
        free(curr->target);        
        Dependency *dep = curr->dependencies;
        Dependency *temp_dep;
        while (dep != NULL){
            temp_dep = dep->next_dep;
            free(dep);            
            dep = temp_dep;
        }
        Action *act = curr->actions;
        Action *temp_act;
        while (act != NULL) {        
            i = 0;        
            while (act->args[i] != NULL) {
                free(act->args[i]);
                i++;
            }            
            free(act->args);
            temp_act = act->next_act; 
            free(act);       
            act = temp_act;
        }            
        free(curr);
        curr = temp;
    }
}

/* Create the rules data structure and return it.
   Figure out what to do with each line from the open file fp
     - If a line starts with a tab it is an action line for the current rule
     - If a line starts with a word character it is a target line, and we will
       create a new rule
     - If a line starts with a '#' or '\n' it is a comment or a blank line 
       and should be ignored. 
     - If a line only has space characters ('', '\t', '\n') in it, it should be
       ignored.
 */
Rule *parse_file(FILE *fp) {
    // TODO
    Rule **first_rule = malloc(sizeof(Rule *));
    *first_rule = NULL;
    Rule *first = NULL;
    Rule *cur = NULL;    
    Rule *child_rule = NULL;
    Rule *prev_rule = NULL;
    Action *new_act = NULL;
    int prev = 0;
    Dependency *new_dependency;
    char line[MAXLINE + 1];
    // Get new line from fp
    while (fgets(line, MAXLINE + 1, fp) != NULL) {
        // Whether ignore the line or not
        if (is_comment_or_empty(line) == 0) {    
        char **result = build_args(line);
        int size = count_size(result);     
            // Check if the first char of line is alphabet                             
            if (isalpha(line[0]) != 0) {     
                // Copy string            
                char tar[strlen(result[0]) + 1];
                strcpy(tar, result[0]); 
                tar[strlen(result[0]) + 1] = '\0';   
                // Check rule                                      
                cur = check_rule(tar, first_rule, child_rule);                                             
                prev_rule = cur;           
                int j;                    
                Dependency *prev_dep = NULL;
                for (j = 2; j < size; j++) {                                    
                    trim(result[j], strlen(result[j])+ 1);
                    new_dependency = create_dependency();
                    if (j == 2) {                    
                        cur->dependencies = new_dependency;                                                        
                        prev_dep = new_dependency;                                       
                    } 
                    else {                        
                        prev_dep->next_dep = new_dependency;
                        prev_dep = new_dependency;                    
                    }       
                    char targ[strlen(result[j]) + 1];
                    strcpy(targ, result[j]);                     
                    targ[strlen(result[j]) + 1] = '\0';             
                    child_rule = create_rule(targ);   
                    if (cur->next_rule == NULL) {                                             
                        cur->next_rule = child_rule; 
                    } else {
                        while (cur->next_rule != NULL) {
                            cur = cur->next_rule;
                        }                       
                        cur->next_rule = child_rule;
                    }               
                    new_dependency->rule = child_rule;
                    cur = child_rule;                                                                                   
                }                                    
                prev = 0;                         
            } else if (line[0] == '\t') {
                /* new action */                    
                Action *prev_act = NULL;                                
                prev_act = new_act;                                      
                for (int y = 0; y < size; y++) {
                    trim(result[y], strlen(result[y]) + 1);
                }                
                new_act = create_action(result, size); 
                if (prev_rule == NULL) {
                    fprintf(stderr, "Error: recipe commences before first target.\n");
                    exit(EXIT_FAILURE);
                } else {
                    new_action(prev_rule, new_act, prev_act, prev);              
                }            
                prev = 1;
            }
            free(result);            
        }         
    }
    first = *first_rule;    
    free(first_rule);        
    return first;
}
