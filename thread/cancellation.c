#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

void cleanup_handler(void *arg) {
	printf(
		"Cleanup handler for thread %ld was called\n", 
		*((long *) arg)
	);
	//clean thread mutexes, used shared variables and so on 
}

void *thread_function(void *arg) {
	
	int seconds = *((int *) arg);
	pthread_t id = pthread_self(); 

	//setting cleanup handler
	pthread_cleanup_push(cleanup_handler, &id);

	if (seconds > 0) {
		printf("Thread sleeping for %d seconds\n", seconds);
		/* sleep() can be treated like a cancellation point
		 * as a replacement of pthread_testcancel(), which responds to
		 * possible cancellation request
		 */
		sleep(seconds);
	}

	//call cleanup handler even though cancellation request was not send
	pthread_cleanup_pop(1);
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

	int option, result;
	int thread_sleep = 0;
	int cancellation_delay = 0;
	void *state;

	while ((option = getopt(argc, argv, ":s:c:")) != -1) {
		switch (option) {
		case 's':
			thread_sleep = atoi(optarg);
			break;
		case 'c':
			cancellation_delay = atoi(optarg);
			break;
		case ':':
			fprintf(
				stderr,
				"Option - %c requires an operand\n",
				optopt
			);
			exit(EXIT_FAILURE);
		case '?':
			fprintf(
				stderr,
				"Unrecognized option -%c\n",
				optopt
			);
			exit(EXIT_FAILURE);
		default:
			fprintf(
				stderr,
				"Usage: %s [-sc]",
				argv[0]
			);
			exit(EXIT_FAILURE);
		}
	}

	pthread_t thread;
	result = pthread_create(&thread, NULL, thread_function, &thread_sleep);

	if (cancellation_delay > 0) {
		printf(
			"Main thread delaying cancellation by %d seconds\n",
			cancellation_delay
		);
		sleep(cancellation_delay);
	}

	/*
	 * cancelling thread, not checking return value due to
	 * final message output
	 */
	pthread_cancel(thread);

	result = pthread_join(thread, &state);
	if (result != 0)
		error("pthread_join");

	if (state == PTHREAD_CANCELED)
		printf("Thread was canceled\n");
	else
		printf("Thread was NOT canceled\n");

	exit(EXIT_SUCCESS);
}

