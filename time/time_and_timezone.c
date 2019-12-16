#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

int error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

void sgettimeofday(struct timeval *tv) {
	if (gettimeofday(tv, NULL) == -1)
		error("gettimeofday");
}

struct tm *slocaltime() {
	time_t seconds = time(NULL);
	if (seconds == -1)
		perror("time");
	struct tm *local_time = localtime(&seconds);
	if (local_time == NULL)
		error("localtime");
	return local_time;
}

char *sasctime(const struct tm *timeptr) {
	char *asc_time = asctime(timeptr);
	if (asc_time == NULL)
		error("asctime");
	return asc_time;
}

void ssetenv(const char *name, const char *value, int overwrite) {
	if (setenv(name, value, overwrite) == -1)
		error("setenv");
}

int main(int argc, char *argv[]) {
	
	int option;

	while ((option = getopt(argc, argv, ":ult:")) != 1) {
		switch(option) {
		case 'u':
		{
			struct timeval time;	
			sgettimeofday(&time);
			printf(
				"Calendar time (seconds since Epoch): %ld secs, %ld microsecs\n",
				(long) time.tv_sec,
				(long) time.tv_usec
			);
			return EXIT_SUCCESS;
		}
		case 'l':
			printf(
				"Local time: %s",
				sasctime(slocaltime())
			);
			return EXIT_SUCCESS;
		case 't':
			ssetenv("TZ", optarg, 1);
			printf(
				"Timezone time: %s",
				sasctime(slocaltime())
			);
			return EXIT_SUCCESS;
		case ':':
			fprintf(
				stderr,
				"Option -%c requires an operand\n",
				optopt
			);
			return EXIT_FAILURE;
		case '?':
			fprintf(
				stderr,
				"Unrecognized option -%c\n",
				optopt
			);
			return EXIT_FAILURE;
		default:
			fprintf(
				stderr,
				"Usage: %s [-u | -l | -t timezone]\n",
				argv[0]
			);
			return EXIT_FAILURE;
		}
	}
}
