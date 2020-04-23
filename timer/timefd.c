#include <errno.h>
#include <sys/timerfd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

	struct itimerspec specification;
	char *p;
	uint64_t expirations, total_expirations, maximum_expirations;
	ssize_t size;

	if (argc < 3 || strcmp(argv[1], "--help") == 0) {
		fprintf(
			stderr,
			"Usage: %s maximum-expirations  %s %s\n",
			argv[0],
			"countdown-seconds[.countdown-miliseconds]",
			"[interval-seconds[.interval.miliseconds]]"
		);
		exit(EXIT_FAILURE);
	}

	//set maximum of expirations on the timer
	maximum_expirations = atoi(argv[1]);

	//set initial countdown
	char *countdown = strdup(argv[2]);

	p = strchr(countdown, '.');
	if (p != NULL)
		*p = '\0';
	specification.it_value.tv_sec = atoi(countdown);
	specification.it_value.tv_nsec = p == NULL ? 0 : atoi(p + 1);

	if (argc > 2) {

		//set optionable interval
		char *interval = strdup(argv[2]);

		p = strchr(interval, '.');
		if (p != NULL)
			*p = '\0';

		specification.it_interval.tv_sec = atoi(interval);
		specification.it_interval.tv_nsec = p == NULL ? 0 : atoi(p + 1);
	} else {
		specification.it_interval.tv_sec = 0;
		specification.it_interval.tv_sec = 0;
	}
	
	//create timer
	int timer = timerfd_create(CLOCK_REALTIME, 0);
	if (timer == -1)
		error("timerfd_create");

	if (timerfd_settime(timer, 0, &specification, NULL) == -1)
		error("timerfd_settime");
	
	//display expiration count
	for (total_expirations = 0; total_expirations < maximum_expirations;) {
		
		size = read(timer, &expirations, sizeof(uint64_t));
		if (size != sizeof(uint64_t))
			error("read");
		total_expirations += expirations;

		printf(
			"Expirations occured: %lu\n", 
			(unsigned long) expirations
		);
	}

	printf("Total expirations: %lu\n", total_expirations);

	exit(EXIT_SUCCESS);
}

