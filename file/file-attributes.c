#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

int set_special_permissions(int permissions) {

	//setuid, setgid and sticky bits
	int const setuid = 04000;
	int const setgid = 02000;
	int const sticky = 01000;

	for (int i = 0; optarg[i] != '\0'; i++)
		switch(optarg[i]) {
		case 'u':
			permissions += setuid;
			break;
		case 'g':
			permissions += setgid;
			break;
		case 's':
			permissions += sticky;
			break;
		}
	return permissions;
}

int set_permissions(
	int permissions,
	int read,
	int write,
	int execute
) {
	for (int i = 0; optarg[i] != '\0'; i++)
		switch(optarg[i]) {
		case 'r':
			permissions += read;
			break;
		case 'w':
			permissions += write;
			break;
		case 'x':
			permissions += execute;
			break;
		}
	return permissions;
}

int main(int argc, char *argv[]) {

	//user permissions
	int const user_read = 0400;
	int const user_write = 0200;
	int const user_execute = 0100;

	//group permissions
	int const group_read = 040;
	int const group_write = 020;
	int const group_execute = 010;

	//other permissions
	int const other_read = 04;
	int const other_write = 02;
	int const other_execute = 01;

	const char *file;
	int permissions = 0;
	int flags = 0;
	int option;

	while ((option = getopt(argc, argv, ":f:x:u:g:o:acijAdtsSu")) != -1) {
		switch(option) {
		case 'f':
			//set the file
			file = optarg;
			break;
		case 'x':
			//set setuid, setgid and sticky bits
			permissions = set_special_permissions(permissions);
		case 'u':
			//set user permissions
			permissions = set_permissions( 
				permissions,
				user_read,
				user_write,
				user_execute
			);
			break;
		case 'g':
			//set other permissions
			permissions = set_permissions(
				permissions,
				group_read,
				group_write,
				group_execute
			);
			break;
		case 'o':
			//set other permissions
			permissions = set_permissions(
				permissions,
				other_read,
				other_write,
				other_execute
			);
			break;
		case 'a':
			//set i-node flag: append only
			flags |= FS_APPEND_FL;
			break;
		case 'c':
			//set i-node flag: enable file compression
			flags |= FS_COMPR_FL;
			break;
		case 'i':
			//set i-node flag: immutable
			flags |= FS_IMMUTABLE_FL;
			break;
		case 'j':
			//set i-node flag: enable data journaling
			flags |= FS_JOURNAL_DATA_FL;
			break;
		case 'A':
			//set i-node flag: don't update file last access time
			flags |= FS_NOATIME_FL;
			break;
		case 'd':
			//set i-node flag: no dump
			flags |= FS_NODUMP_FL;
			break;
		case 't':
			//set i-node flag: no tail packing
			flags |= FS_NOTAIL_FL;
			break;
		case 's':
			//set i-node flag: secure deletion
			flags |= FS_SECRM_FL;
			break;
		case 'S':
			//set i-node flag: synchronous file updates
			flags |= FS_SYNC_FL;
			break;
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
				"Usage: %s -f file [-u [rwx]] [-g [rwx]] [-o [rwx]]\n",
				argv[0]
			);
			exit(EXIT_FAILURE);
		}
	}

	//changing file permissions
	if (chmod(file, permissions) == -1)
		error("chmod");
	//changing file i-node flags
	int file_descriptor = open(file, O_RDONLY);
	if (file_descriptor == -1)
		error("open");
	if (ioctl(file_descriptor, FS_IOC_SETFLAGS, &flags) == -1)
		error("ioctl");
	return EXIT_SUCCESS;
}
