#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <time.h>
#include "socket.h"  
#include "message.h"
#include "controller.h"

#define MAXFD(x,y) ((x) >= (y)) ? (x) : (y)

int main(int argc, char *argv[]){
	int port;
	struct cignal cig;
	// A buffer to store a serialized message
	char *cig_serialized = malloc(sizeof(char)*CIGLEN);
	// An array to registered sensor devices
	int device_record[MAXDEV] = {0};
	
	if(argc == 2){
		port = strtol(argv[1], NULL, 0);
	} else{
		fprintf(stderr, "Usage: %s port\n", argv[0]);
		exit(1);
	}

	int gatewayfd = set_up_server_socket(port);
	printf("\nThe Cignal Gateway is now started on port: %d\n\n", port);
	int peerfd;
	
	/* TODO: Implement the body of the server.  
	 *
	 * Use select so that the server process never blocks on any call except
	 * select. If no sensors connect and/or send messsages in a timespan of
	 * 5 seconds then select will return and print the message "Waiting for
	 * Sensors update..." and go back to waiting for an event.
	 * 
	 * The server will handle connections from devices, will read a message from
	 * a sensor, process the message (using process_message), write back
	 * a response message to the sensor client, and close the connection.
	 * After reading a message, your program must print the "RAW MESSAGE"
	 * message below, which shows the serialized message received from the *
	 * client.
	 * 
	 *  Print statements you must use:
     * 	printf("Waiting for Sensors update...\n");
	 * 	printf("RAW MESSAGE: %s\n", YOUR_VARIABLE);
	 */

	// TODO implement select loop

	// Suppress unused variable warning.  The next 5 ilnes can be removed 
	// after the variables are used.
	// (void)gatewayfd;
	// (void)peerfd;
	// (void)cig;
	// (void)device_record;
	// (void)cig_serialized;	
	int max_fd = gatewayfd;
	fd_set all_fds;	
	FD_ZERO(&all_fds);
	FD_SET(gatewayfd, &all_fds);

	struct timeval time;
	

	while(1) {
		// Set time-out to 5 seconds.
		time.tv_sec = 5;
		time.tv_usec = 0;
		fd_set listen_fds = all_fds;
		int nready = select(max_fd + 1, &listen_fds, NULL, NULL, &time);
		if (nready == -1) {
			perror("server: select");
			exit(EXIT_FAILURE);
		} else if (nready == 0) {
			// Time-out
			printf("Waiting for Sensors update...\n");			
			continue;
		}	
		// iterating over fd set	
		for (int i = 0; i <= max_fd; i++) {
			// check fd is in listen_fds
			if (FD_ISSET(i, &listen_fds)) {				
				if (i == gatewayfd) {									
					peerfd = accept_connection(gatewayfd);					
					if (peerfd < 0) {
						perror("accept() failed");							
						exit(EXIT_FAILURE);
					}
					FD_SET(peerfd, &all_fds);				
					max_fd = MAXFD(max_fd, peerfd);																			
				} else {	
					// read from sensor							
					int num_read = read(i, cig_serialized, CIGLEN);					
					if (num_read < 0) {
						perror("read");
						exit(EXIT_FAILURE);					
					}	
					// convert message to cignal				
					unpack_cignal(cig_serialized, &cig);
					// Change orignial struct cig to string
					cig_serialized[num_read] = '\0';						
					printf("RAW MESSAGE: %s\n", cig_serialized);		
					// Process message
					int result = process_message(&cig, device_record);
					if (result == -1) {
						// invalid message discard message and close connection
						close(i);
						FD_CLR(i, &all_fds);
						continue;
					} else {
						// Valid message
						// Convert processed struct cig to string representation
						cig_serialized = serialize_cignal(cig);
						// Change processed struct cig to string
						cig_serialized[num_read] = '\0';	
						// printf("Writing: %s\n", cig_serialized);	
						int num_written = write(i, cig_serialized, CIGLEN);
						if (num_written < 0) {
							perror("write");
							exit(EXIT_FAILURE);						
						}
					}	
					// close and clear fd
					close(i);					
					FD_CLR(i, &all_fds);
					// Change max_fd	
					// Never reduce max_fd														
				}
			}
		}	
	}
	return 1;
}
