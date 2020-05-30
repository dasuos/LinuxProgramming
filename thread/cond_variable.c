#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t thread_detached = PTHREAD_COND_INITIALIZER;

static int active_threads = 0;
static int detached_threads = 0;

enum thread_state {
	ACTIVE,
	DETACHED,
	DISPLAYED
};

static struct {
	pthread_t id;
	enum thread_state state;
	int sleep;
} *thread;

void error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

void *thread_function(void *arg) {

	int id =  *((int *) arg);
	int result;

	printf("Threat sleeping for %d seconds\n", thread[id].sleep); 
	sleep(thread[id].sleep);
	
	result = pthread_mutex_lock(&mutex);
	if (result != 0)
		error("pthread_mutex_lock");
	
	//detaching thread
	if (pthread_detach(pthread_self()) != 0)
		error("pthread_detach");
	thread[id].state = DETACHED;
	detached_threads++;

	result = pthread_mutex_unlock(&mutex);
	if (result != 0)
		error("pthread_mutex_unlock");

	//signaling change of state
	result = pthread_cond_signal(&thread_detached);
	if (result != 0)
		error("pthread_cond_signal");
	
	return NULL;
}

int main(int argc, char *argv[]) {

	int *id;
	int result;
	int i;

	if (argc < 2 || strcmp(argv[1],  "--help") == 0) {
		fprintf(
			stderr,
			"Usage: %s thread_sleep_seconds...",
			argv[1]
		);
		exit(EXIT_FAILURE);
	}

	//allocate memory for thread structures
	int total_threads = argc - 1;
	thread = calloc(total_threads, sizeof(*thread));
	if (thread == NULL)
		error("calloc");

	//initialize thread structures and create threads
	for (i = 0; i < total_threads; i++) {
		
		id = (int *) malloc(sizeof(int));
		if (id == NULL)
			error("malloc");
		*id = i;
		
		thread[i].sleep = atoi(argv[i + 1]);
		thread[i].state = ACTIVE;
		result = pthread_create(
			&thread[i].id,
			NULL,
			thread_function,
			(void *) id
		);
		if (result != 0)
			error("pthread_create");
	}
	active_threads = total_threads;
	
	while (active_threads > 0) {

		result = pthread_mutex_lock(&mutex);
		if (result != 0)
			error("pthread_mutex_lock");
		
		//wait on the condition variable and continue
		while (detached_threads == 0) {
			result = pthread_cond_wait(&thread_detached, &mutex);
			if (result != 0)
				error("pthread_cond_wait");
		}

		//display detached thread
		for (i = 0; i < total_threads; i++) {
			if (thread[i].state == DETACHED) {
				active_threads--;
				printf(
					"Thread %ld detached (active threads: %d)\n",
					thread[i].id,
					active_threads
				);
				thread[i].state = DISPLAYED;
			}
		}

		result = pthread_mutex_unlock(&mutex);
		if (result != 0)
			error("pthread_mutex_unlock");
	}

	return EXIT_SUCCESS;
}

