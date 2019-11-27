#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_SOURCES 10

int error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

int sopen(const char *path, int flags, ... /* mode_t mode */) {
	
	va_list arguments;

	va_start(arguments, flags);
		mode_t mode = va_arg(arguments, mode_t);
	va_end(arguments);
	int descriptor = mode == 0
		? open(path, flags) : open(path, flags, mode);
	if (descriptor == -1)
		error("open");
	return descriptor;
}

void spread(int descriptor, void *buffer, size_t count, off_t offset) {
	if (pread(descriptor, buffer, count, offset) == -1)
	       error("pread");	
}

size_t swritev(int descriptor, const struct iovec *iov, int iovcnt) {
	ssize_t count = writev(descriptor, iov, iovcnt);
	if (count == -1)
		error("writev");
	return count;
}

void sclose(int descriptor) {
	if (close(descriptor) == -1)
		error("close");
}

void sstat(const char *path, struct stat *status) {
	if (stat(path, status) == -1)
		error("stat");
}

void *smalloc(size_t size) {
	void *buffer = malloc(size);
	if (buffer == NULL)
		error("malloc");
	return buffer;
}

int main(int argc, char *argv[]) {
	
	int option; 
	int destination = -1;
	int source_count = 0;
	char *sources[MAX_SOURCES];
	int i;

	while ((option = getopt(argc, argv, ":o:a:")) != -1) {
		switch (option) {
		case 'o':
			//set the destination
			if (optarg[0] != '-') {
				destination = sopen(
					optarg, 
					O_CREAT | O_WRONLY | O_APPEND, 
					S_IRUSR | S_IWUSR | S_IRGRP | 
					S_IWGRP | S_IROTH | S_IWOTH
				);
				break;
			}
			fprintf(stderr, "Option -o requires an operand\n");
                        return EXIT_FAILURE;
		case 'a':
			//set the sources
			for (i = optind - 1; i < argc; i++)
				if (argv[i][0] != '-') {
					if (source_count > MAX_SOURCES) {
						fprintf(
							stderr,
							"Maximum amount of files to append is %d\n",
							MAX_SOURCES
						);
						return EXIT_FAILURE;
					}
					sources[source_count++] = argv[i];
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

	if (destination == -1 || source_count == 0) {
		fprintf(
			stderr,
			"Usage: %s -o destination -a source1 source2 ...\n",
			argv[0]
		);
		return EXIT_FAILURE;
	}
	
	int source;
	struct stat status;
	char *buffer[source_count];
	struct iovec iov[source_count];
	int bytes_read = 0;

	for (i = 0; i < source_count; i++) {

		source = sopen(sources[i], O_RDONLY);
		
		//allocate a buffer based on source size
		sstat(sources[i], &status);
		buffer[i] = smalloc(status.st_size);

		//atomically read a source and store in a buffer
		spread(source, buffer[i], status.st_size, 0);
		
		iov[i].iov_base = buffer[i];
		iov[i].iov_len = status.st_size;
		bytes_read += status.st_size;

		sclose(source);
	}
	
	//write atomically to a destination
	ssize_t bytes_written = swritev(destination, iov, source_count);

	if (bytes_written < bytes_read)
		printf("Written fewer bytes than read\n");
	printf("Total bytes read: %ld, appended: %ld\n", 
		(long) bytes_read, (long) bytes_written);

	return EXIT_SUCCESS;
}

