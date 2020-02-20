#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <unistd.h>

int error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

long limit(int name) {
	
	long limit = sysconf(name);
	errno = 0;

	if (errno != 0)
		perror("sysconf");
	return limit;
}

struct utsname information() {
	
	struct utsname uts;

	if (uname(&uts) == -1)
		perror("uname");
	return uts;
}

void print_limit(char *message, int name) {

	long result = limit(name);

	printf("%s ", message);
	result != -1
		? printf("%ld\n", result) 
		: printf("indeterminate\n");
}

int main(int argc, char *argv[]) {

	printf("System limits:\n\n");

	print_limit(
		"Maximum bytes for arguments argv and environ in exec(): ",
		_SC_ARG_MAX
	);
	print_limit(
		"Maximum size of a login name (including \\0):            ",
		_SC_LOGIN_NAME_MAX
	);
	print_limit(
		"Maximum number of opened file descriptors for a process:",
	       _SC_OPEN_MAX
	);
	print_limit(
		"Maximum number of supplementary groups for a process:   ",
		_SC_NGROUPS_MAX
	);
	print_limit(
		"Size of a virtual memory page:                          ",
		_SC_PAGE_SIZE
	);
	print_limit(
		"Maximum number of distinct realtime signals:            ",
		_SC_RTSIG_MAX
	);
	print_limit(
		"Maximum number of queued realtime signals:              ",
		_SC_SIGQUEUE_MAX
	);
	print_limit(
		"Maximum number of opened stdio streams at one time:     ",
		_SC_STREAM_MAX
	);

	printf("\nSystem information:\n\n");

	struct utsname uts = information();

	printf("Node name:   %s\n", uts.nodename);
	printf("System name: %s\n", uts.sysname);
	printf("Release:     %s\n", uts.release);
	printf("Version:     %s\n", uts.version);
	printf("Machine:     %s\n", uts.machine);
	#ifdef _GNU_SOURCE
		printf("Domain name: %s\n", uts.domainame);
	#endif	

	return EXIT_SUCCESS;
}

