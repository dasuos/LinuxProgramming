#define _XOPEN_SOURCE 500
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_LENGTH (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))
#define WATCH_COUNT 8192

static int directory_count = 0;
static char **directories = NULL;
static int directory_size = 0;
static const int DIRECTORY_INCREMENT = 1000;

void error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

static int traverse_directories(
	const char *path,
	const struct stat *information,
	int type,
	struct FTW *ftw
) {
	
	//skip if not directory
	if (!S_ISDIR(information->st_mode))
		return 0;

	//allocate memory for directory path
	if (directory_count >= directory_size) {
		directory_size += DIRECTORY_INCREMENT;
		directories = realloc(
			directories, 
			directory_size * sizeof(char *)
		);
		if (directories == NULL)
			error("realloc");	
	}

	//add directory into list
	directories[directory_count] = strdup(path);
	directory_count++;

	return 0;
}

int main(int argc, char *argv[]) {

	int watch;
	char buffer[BUFFER_LENGTH] __attribute__((aligned(8)));
	ssize_t bytes;
	char *p;
	struct inotify_event *event;

	if (argc != 3 || strcmp(argv[1], "--help") == 0) {
		fprintf(stderr, "Usage: %s file seconds\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	//opening log file and setting countdown
	FILE *log = fopen(argv[1], "a");
	time_t countdown = time(NULL) + atoi(argv[2]);
	
	//travese directories and add them into list
	nftw(".", traverse_directories, 20, 0);

	//create inotify instance
	int inotify = inotify_init();
	if (inotify == -1)
		error("inotify_init");

	//add directories for all events
	for (int i = 0; directory_count > i; i++) {
		watch = inotify_add_watch(
			inotify,
			directories[i],
			IN_ALL_EVENTS
		);
		if (watch == -1)
			error("inotify_add_watch");
		printf("Directory %s added into watch list\n", directories[i]);
	}

	//read directory events
	while (time(NULL) < countdown) {
		bytes = read(inotify, buffer, BUFFER_LENGTH);
		if (bytes == 0) {
			fprintf(stderr, "Reading from inotify failed");
			exit(EXIT_FAILURE);
		}
		if (bytes == -1)
			error("read");
		
		//process and print directory events
		for (p = buffer; p < buffer + bytes; ) {
			event = (struct inotify_event *) p;
			
			//log event
			fwrite("abc", 1, sizeof("abc"), log);

			p += sizeof(struct inotify_event) + event->len;
		}
	}
	
	fclose(log);
	exit(EXIT_SUCCESS);
}

