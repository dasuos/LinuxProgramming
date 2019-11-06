#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define MAX_SOURCES 10

int error(char *message) {
	perror(message);
	return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
	
	int option, i;
	int source, destination = -1, source_number = 0;
	struct stat status;
	ssize_t loaded = 0, written;
	char *sources[MAX_SOURCES];

	while ((option = getopt(argc, argv, ":o:a:")) != -1) {
		switch (option) {
		case 'o':
			//set the destination
			if (optarg[0] != '-') {
				destination = open(
					optarg, 
					O_CREAT | O_WRONLY | O_APPEND, 
					S_IRUSR | S_IWUSR | S_IRGRP | 
					S_IWGRP | S_IROTH | S_IWOTH
				);
				if (destination == -1)
					error("open");
				break;
			}
			fprintf(stderr, "Option -o requires an operand\n");
                        return EXIT_FAILURE;
		case 'a':
			//set the sources
			for (i = optind - 1; i < argc; i++)
				if (argv[i][0] != '-') {
					if (source_number > MAX_SOURCES) {
						fprintf(
							stderr,
							"Maximum amount of files to append is %d\n",
							MAX_SOURCES
						);
						return EXIT_FAILURE;
					}
					sources[source_number++] = argv[i];
				} else {
					optind = i;
					break;
				}
			break;
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
				"Usage: %s -o destination -a source1 source2 ...\n", 
				argv[0]
			);
			return EXIT_FAILURE;
		}
	}

	if (destination == -1 || source_number == 0) {
		fprintf(
			stderr,
			"Usage: %s -o destination -a source1 source2 ...\n",
			argv[0]
		);
		return EXIT_FAILURE;
	}

	struct iovec iov[source_number];
	char *buffer[source_number];

	for (i = 0; i < source_number; i++) {

		source = open(sources[i], O_RDONLY);
		if (source == -1)
			return error("open");
		
		//allocate a buffer based on source size
		if (stat(sources[i], &status) == -1)
			perror("stat");
		buffer[i] = malloc(status.st_size);
		if (buffer[i] == NULL)
			return error("malloc");

		//atomically read a source and store in a buffer
		if (pread(source, buffer[i], status.st_size, 0) == -1)
			return error("read");
		iov[i].iov_base = buffer[i];
		iov[i].iov_len = status.st_size;
		loaded += status.st_size;

		close(source);
	}
	
	//write atomically to a destination
	written = writev(destination, iov, source_number);

	if (written == -1)
		error("writev");
	if (written < loaded)
		printf("Written fewer bytes than read\n");
	printf("Total bytes read: %ld, appended: %ld\n", 
		(long) loaded, (long) written);

	return EXIT_SUCCESS;
}

