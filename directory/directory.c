#include <dirent.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

char *path(char *directory_name) {
	
	size_t path_size = sizeof(directory_name) + 2;
	char *path = malloc(path_size);

	snprintf(path, path_size, "./%s", directory_name);
	return path;
}

int main(int argc, char *argv[]) {

	int option;

	while ((option = getopt(argc, argv, ":m:r:l")) != -1) {
		switch (option) {
		case 'm': {
			//make a directory
			if (mkdir(path(optarg), S_IRWXU | S_IRWXG | S_IXOTH) == -1)
				error("mkdir");
			exit(EXIT_FAILURE);
		}
		case 'r': {
			//remove a directory
			if (rmdir(path(optarg)) == -1)
				error("rmdir");
			exit(EXIT_FAILURE);
		}
		case 'l': {
			
			DIR *directory_stream = opendir(".");

			if (directory_stream == NULL) {
				fprintf(stderr, "Opening current directory failed");
				exit(EXIT_FAILURE);
			}

			struct dirent *entry;

			//print directory content
			for(;;) {
				errno = 0;

				entry = readdir(directory_stream);
				if (entry == NULL)
					break;
				
				if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
					continue;

				printf("%s\n", entry->d_name);
			}

			if (errno != 0)
				error("readdir");
			if (closedir(directory_stream) == -1)
				error("closedir");

			exit(EXIT_FAILURE);
		}
		case ':':
			fprintf(
				stderr,
				"Option -%c requires an operand\n",
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
				"Usage: %s [-m dir | -r dir | -l]",
				argv[0]
			);
			exit(EXIT_FAILURE);
		}
	}
}

