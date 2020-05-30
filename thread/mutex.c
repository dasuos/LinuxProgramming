#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static volatile int shared_loops = 0;

void error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

void *thread_function(void *arg) {
	
	int with_mutex = *((int *) arg);
	int result, i, j;

	for (i = 0; i < 1000000; i++) {
		
		if (with_mutex) {
			//lock mutex
			result = pthread_mutex_lock(&mutex);
			if (result != 0)
				error("pthread_mutex_lock");
		}

		//critical section using shared variable
		j = shared_loops;
		j++;
		shared_loops = j;

		if (with_mutex) {
			//unlock mutex
			result = pthread_mutex_unlock(&mutex);
			if (result != 0)
				error("pthread_mutex_unlock");
		}
	}
	return NULL;
}

int main(int argc, char *argv[]) {

	pthread_t thread_one, thread_two;
	int option, result;
	int with_mutex = 0;

	while ((option = getopt(argc, argv, ":mc")) != -1) {
		switch (option) {
		case 'm':
			with_mutex = 1;
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
				"Usage: %s [-m]",
				argv[0]
			);
			exit(EXIT_FAILURE);
		}
	}

	result = pthread_create(&thread_one, NULL, thread_function, &with_mutex);
	if (result != 0)
		error("pthread_create");
	result = pthread_create(&thread_two, NULL, thread_function, &with_mutex);
	if (result != 0)
		error("pthread_create");

	result = pthread_join(thread_one, NULL);
	if (result != 0)
		error("pthread_join");
	result = pthread_join(thread_two, NULL);
	if (result != 0)
		error("pthread_join");

	printf("Loop count: %d\n", shared_loops);

	exit(EXIT_SUCCESS);
}

