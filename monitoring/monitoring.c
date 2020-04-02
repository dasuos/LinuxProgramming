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
	if (
		!S_ISDIR(information->st_mode) ||
		strcmp(&path[ftw->base], ".") == 0
	)
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

void log_event(char *path, struct inotify_event *event) {

	char *log;

	//set log message by event type
	if (event->mask & IN_ACCESS)
		log = "IN_ACCESS - file was accessed";
	else if (event->mask & IN_ATTRIB)
		log = "IN_ATTRIB - file metadata changed";
	else if (event->mask & IN_CLOSE_WRITE)
		log = "IN_CLOSE_WRITE - file opened for writing was closed";
	else if (event->mask & IN_CLOSE_NOWRITE)
		log = "IN_CLOSE_NOWRITE - file opened read-only was closed";
	else if (event->mask & IN_CREATE)
		log = "IN_CREATE - file/directory created";
	else if (event->mask & IN_DELETE)
		log = "IN_DELETE - file/directory deleted";
	else if (event->mask & IN_DELETE_SELF)
		log = "IN_DELETE_SELF - file/directory was itself deleted";
	else if (event->mask & IN_MODIFY)
		log = "IN_MODIFY - file was modified";
	else if (event->mask & IN_MOVE_SELF)
		log = "IN_MOVE_SELF - file/directory was itself moved";
	else if (event->mask & IN_MOVED_FROM)
		log = "IN_MOVED_FROM - file moved out of directory";
	else if (event->mask & IN_MOVED_TO)
		log = "IN_MOVED_TO - file moved into directory";
	else if (event->mask & IN_OPEN)
		log = "IN_OPEN - file was opened";

	//format and save log message
	FILE *log_file = fopen(path, "a");
	fprintf(
		log_file, 
		"%s wd = %d", 
		event->mask & IN_ISDIR ? "[DIRECTORY]" : "[FILE]",
		event->wd
	);
	if (event->len)
		fprintf(log_file, ", name = %s", event->name);
	fprintf(log_file, ": %s\n", log);
	fclose(log_file);
}

int main(int argc, char *argv[]) {

	int watch;
	char buffer[BUFFER_LENGTH] __attribute__((aligned(8)));
	ssize_t bytes;
	struct inotify_event *event;
	size_t logged = 0;

	if (argc != 2 || strcmp(argv[1], "--help") == 0) {
		fprintf(stderr, "Usage: %s file \n", argv[0]);
		exit(EXIT_FAILURE);
	}

	char *log_file = argv[1];	

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

	for (;;) {		
		logged = 0;

		//read directory events 
		bytes = read(inotify, buffer, BUFFER_LENGTH);
		
		if (bytes == 0) {
			fprintf(stderr, "Reading from inotify failed");
			exit(EXIT_FAILURE);
		}
		if (bytes == -1)
			error("read");

		//process and log directory events
		while (bytes > logged) {

			event = (struct inotify_event *) &buffer[logged];
			log_event(log_file, event);

			logged += sizeof(struct inotify_event) + event->len;
		}
	}	

	exit(EXIT_SUCCESS);
}

