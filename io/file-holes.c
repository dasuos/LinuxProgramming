#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

int sopen(const char *path, int flags, ... /* mode_t mode */) {
	
	va_list arguments;
	mode_t mode;
	int descriptor;
	
	va_start(arguments, flags);
		mode = va_arg(arguments, mode_t);
	va_end(arguments);
	descriptor = mode == 0
		? open(path, flags) : open(path, flags, mode);
	if (descriptor == -1)
		error("open");
	return descriptor;
}

void sread(int descriptor, void *buffer, size_t count) {
	if (read(descriptor, buffer, count) == -1)
		error("read");
}

void sclose(int descriptor) {
	if (close(descriptor) == -1)
		error("close");
}

ssize_t swrite(int descriptor, const void *buffer, size_t count) {
	ssize_t written = write(descriptor, buffer, count);
	if (written == -1)
		error("write");
	return written;
}

off_t slseek(int descriptor, off_t offset, int whence) {
	offset = lseek(descriptor, offset, whence);
	if (offset == -1)
		error("lseek");
	return offset;
}

void *smalloc(size_t size) {
	void *buffer = malloc(size);
	if (buffer == NULL)
		error("malloc");
	return buffer;
}

int main(int argc, char *argv[]) {

	if (argc != 3 || strcmp(argv[1], "--help") == 0) {
                fprintf(stderr, "Usage: %s old-file new-file\n", argv[0]);
                return EXIT_FAILURE;
        }
	
	int flags = O_CREAT | O_WRONLY | O_TRUNC;
	mode_t permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | 
		S_IROTH | S_IWOTH;
	
	//open an old file and create a new one
	int old = sopen(argv[1], O_RDONLY);
	int new = sopen(argv[2], flags, permissions);
	
	//allocate a buffer based on old file size
	ssize_t size = slseek(old, 0, SEEK_END);
	char *buffer = smalloc(size);
	slseek(old, 0, SEEK_SET);

	//copy data and create corresponding holes
	sread(old, buffer, size);
	size_t hole = 0;
	for (int i = 0; i < size; i++) {
		if (buffer[i] == '\0')
			hole++;
		else {
			if (hole) {
				slseek(new, hole, SEEK_CUR);
				hole = 0;
			}
			swrite(new, &buffer[i], 1);
		}
	}

	sclose(old);
	sclose(new);

	return EXIT_SUCCESS;
}

