#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "pmake.h"

/*
 * Compare the modified time of target and dependency.
 * 	-If target is more recent, return -1.
 * 	-If dependency is more recent, return 1.
 * 	-If modified times are equivalent, return 0.
 */
int compare(struct stat target, struct stat dep) {
	int result = 0;
	long target_sec = (long)(target.st_mtim).tv_sec;
	long dep_sec = (long)(dep.st_mtim).tv_sec;
	long target_nsec = (long)(target.st_mtim).tv_nsec;
	long dep_nsec = (long)(dep.st_mtim).tv_nsec;
	if (target_sec > dep_sec) {
		result = -1;
	} else if (target_sec < dep_sec) {
		result = 1;
	} else {
		if (target_nsec > dep_nsec) {
			result = -1;
		}
		else {
			result = 1;
		}
	}
	return result;
}

/*
 * Calculate total length of the strings inside string array.
 */
int count_total(char **list) {
	int i = 0;
	int result = 0;
    while (list[i] != NULL) {
        result += strlen(list[i]);
        i++;
    }
    result++;
    return result;
}

/*
 * Execute an action by using fork and exec to the run the command specified by an action. 
 */
void ppmake(Rule *target_rule) {
	Action *act = target_rule->actions;
	int freq = 0;		
	int i, r;
	while (act != NULL) {  			
		freq++;	
		act = act->next_act;				
	} 	
	act = target_rule->actions;
	if (freq != 0 ) {				
		for (i = 0; i < freq; i++) {
			r = fork();			
			if (r == -1) {
		        perror("fork:");
		        exit(1);
		    } else if (r == 0) {  		    	
		    	char *param = act->args[0];				    			    	 		  		    	
	    		int num = count_size(act->args);
		    	int total = count_total(act->args);
		    	if (target_rule->dependencies != NULL && access(act->args[num - 1], F_OK) == -1) {
		    		fprintf(stderr, "Error: No rule to make target '%s', needed by '%s'.\n", 
		    			act->args[num - 1], target_rule->target);
		    		exit(EXIT_FAILURE);
		    	} 
		    	total = total + num - 1;
		    	char buf[total];
		    	char *buff = args_to_string(act->args, buf, total);	
		    	buff[total] = '\0';	    				    	
		    	printf("%s\n", buff);			    	
		    	execvp(param, act->args);		    		
		    	perror("exec");
		    	exit(EXIT_FAILURE);
		    }
		    act = act->next_act;
		}
		for (i = 0; i < freq; i ++) {
			pid_t pid;
		    int status;
		    if( (pid = wait(&status)) == -1) {
		        perror("wait");
		    } else {
		        if (WIFEXITED(status)) {
		            int exitcode = WEXITSTATUS(status);		            
					if (exitcode == 1) {						
					    // perror("status");
					    exit(EXIT_FAILURE);		
		            } 
		        }
		    }  
		}   		 
	}
}

/*
 * When pmake is run with the -p option, a child should be created to update each 
 * dependency. The parent will create one child process for each dependency and 
 * after it has created all of them, the parent will wait for all children to 
 * terminate successfully before determining whether it needs to execute the actions.
 */
void p_make(char *target, Rule *rules, Rule **first, int *total_count) {
	struct stat target_stat;
    struct stat dep_stat;  
    int result = 0;        
    int count = 0;
    Rule *target_rule = rules;
    if (target != NULL) {
    	target_rule = search_rule(rules, target);
    }   
    if (target_rule == NULL) {
    	fprintf(stderr, "Error: target does no exist\n");
    	exit(EXIT_FAILURE);
    }
    Dependency *dep_freq = target_rule->dependencies;
    while (dep_freq != NULL) {    	
    	p_make(dep_freq->rule->target, rules, first, total_count);
    	dep_freq = dep_freq->next_dep;
    }      
	dep_freq = target_rule->dependencies;    	
	int freq = 0;				
	int i, r;
	while (dep_freq != NULL) {  			
		freq++;				
		dep_freq = dep_freq->next_dep;				
	} 	
	dep_freq = target_rule->dependencies;
	if (freq != 0) {				
		for (i = 0; i < freq; i++) {			
			r = fork();					
			if (r == -1) {
		        perror("fork:");
		        exit(1);
		    } else if (r == 0) { 		    	
				// check target file exists or not
				if (target_rule->actions != NULL) {
					if (lstat(target_rule->target, &target_stat) != -1) {           
				    // target file exists.  
				    /* compare modified time */							    
				        if (lstat(dep_freq->rule->target, &dep_stat) == -1) {
				           perror("lstat");
				           exit(1);
				        }
				        result = compare(target_stat, dep_stat);				        
				        // if result is 1, it means dependency is more recent.     	        
				        if (result == 1) {
				        	count++;	        	
				        }					       
					} else {
						count++;
					}						
				}			
				free(total_count);			
				clean(*first);				
				free(first);								
				if (count > 0) {
					// Dependency is more recent
			    	exit(3);						    			
			    } else if (count == 0) {
			    	// target is more recent
			    	exit(0);
			    }
			} 
			dep_freq = dep_freq->next_dep;				
		}								
		for (i = 0; i < freq; i++) {			
			pid_t pid;
		    int status;
		    if( (pid = wait(&status)) == -1) {
		        perror("wait");
		    } else {
		        if (WIFEXITED(status)) {
		            int exitcode = WEXITSTATUS(status);		            
		            if (exitcode == 0) {
		            	count += 0;		         
		            } else if (exitcode == 3) {
		            	count++;
		       //      } else if (exitcode == 1) {
					    // perror("status");
					    // exit(EXIT_FAILURE);		
		            }
		        }
		    }  
		}   						
    } else if (target_rule->actions != NULL) {
    	count++;
    }       
    if (count > 0) {    
    	// Dependency more recent	    		    
    	(*total_count)++;   
    	ppmake(target_rule); 	    	  	   	   
    }
}

/*
 * Compare last modified time, execute the action if it is needed and
 * update dependencies.
 */
void do_make(char *target, Rule *rules, int *total_count) {
	struct stat target_stat;
    struct stat dep_stat;  
    int result = 0;        
    int count = 0;   
    Rule *target_rule = rules;
    if (target != NULL) {
    	target_rule = search_rule(rules, target);
    }    
    if (target_rule == NULL) {
    	fprintf(stderr, "Error: target does no exist\n");
    	exit(EXIT_FAILURE);
    }
    Dependency *dep = target_rule->dependencies;
    while (dep != NULL) {
    	// (*total_count)++;
    	// (*rule_count)++;
    	do_make(dep->rule->target, rules, total_count);
    	dep = dep->next_dep;
    }       
    if (target_rule->actions != NULL) {
		// check target file exists or not
		if (lstat(target_rule->target, &target_stat) != -1) {           
	    // target file exists.  
	    /* compare modified time */		
		    dep = target_rule->dependencies; 		    	    	
		    while (dep != NULL) {
		        if (lstat(dep->rule->target, &dep_stat) == -1) {
		           perror("lstat");
		           exit(EXIT_FAILURE);
		        }
		        result = compare(target_stat, dep_stat);		        	        	       
		        if (result == 1) {
		        	count++;	        	
		        }
		        dep = dep->next_dep;
		    } 		    
		} else {
			count++;
		}
	}     		
	if (count > 0) {    		
    	(*total_count)++;   
    	ppmake(target_rule); 	    
    }	
}

/*
 * Evaluate a makefile rule
 */
void run_make(char *target, Rule *rules, int pflag) {
    // TODO    
    Rule **first = malloc(sizeof(Rule *));
    *first = rules;
    int *total_count = malloc(sizeof(int));
    *total_count = 0;      
    if (pflag == 0) {
    	do_make(target, rules, total_count);      	
	    if (*total_count == 0) {
	    	printf("'pmake' is up-to-date.\n");
	    }  
    } else {
    	p_make(target, rules, first, total_count);    
    	if (*total_count == 0) {
    		printf("'pmake' is up-to-date.\n");	
    	}
    }    
    free(total_count);    
    clean(*first);
    free(first);
}

