#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int error(char *message) {
	perror(message);
	return EXIT_FAILURE;
}

int main(int argc, char *argv[]) {
	
	int old, new, flags, i;
	mode_t permissions;
	ssize_t size, hole = 0;
	char *buffer;
	
	if (argc != 3 || strcmp(argv[1], "--help") == 0) {
                fprintf(stderr, "Usage: %s old-file new-file\n", argv[0]);
                return EXIT_FAILURE;
        }
	
	flags = O_CREAT | O_WRONLY | O_TRUNC;
	permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | 
		S_IROTH | S_IWOTH;
	
	//open an old file and create a new one
	old = open(argv[1], O_RDONLY);
	if (old == -1)
		return error("open");
	new = open(argv[2], flags, permissions);
        if (new == -1)
                return error("open");
	
	//allocate buffer based on old file size
	size = lseek(old, 0, SEEK_END);
	if (size == -1)
		return error("seek");
	buffer = malloc(size);
	if (buffer == NULL)
		return error("malloc");
	if (lseek(old, 0, SEEK_SET) == -1)
		return error("seek");

	//copy data and create corresponding holes
	if (read(old, buffer, size) == -1)
		return error("read");
	for (i = 0; i < size; i++) {
		if (buffer[i] == '\0')
			hole++;
		else {
			if (hole) {
				if (lseek(new, hole, SEEK_CUR) == -1)
					return error("seek");
				hole = 0;
			}
			if (write(new, &buffer[i], 1) == -1)
				return error("write");
		}
	}

	free(buffer);
	if (close(old) == -1)
		return error("close");
	if (close(new) == -1)
		return error("close");

	return EXIT_SUCCESS;
}

