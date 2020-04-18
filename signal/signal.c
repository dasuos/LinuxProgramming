#define _GNU_SOURCE
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_ARGUMENTS 3

void error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

void display_usage(char *argv[]) {
	fprintf(
		stderr,
		"Usage: %s -l | -s pid sig | -r pid sig value\n",
		argv[0]
	);
	exit(EXIT_FAILURE);
}

char *signal_description(int signal, int code) {

	switch (signal) {
	case SIGBUS:
		return "[SIGBUS] memory access errors";
	case SIGCHLD:
		return "[SGCHLD] child process terminates";
	case SIGFPE:
		return "[SIGFPE] arithmetic error occurs";
	case SIGILL:
		return "[SIGILL] illegal machine-language instruction";
	case SIGSEGV:
		return "[SIGSEGV] invalid memory reference";
	case SIGTRAP:
		return "[SIGTRAP] debugger breakpoints";
	}

	switch (code) {
	case SI_ASYNCIO:
		return "[SI_ASYNCIO] completion of an asynchronous I/O";
	case SI_KERNEL:
		return "[SI_KERNEL] sent by the kernel";
	case SI_MESGQ:
		return "[SI_MESGQ] message arrival on POSIX message queue";
	case SI_QUEUE:
		return "[SI_QUEUE] realtime signal";
	case SI_SIGIO:
		return "[SI_SIGIO] SIGIO signal";
	case SI_TIMER:
		return "[SI_TIMER] expiration of a POSIX timer";
	case SI_TKILL:
		return "[SI_TKILL] user process via tkill() or tgkill()";
	case SI_USER:
		return "[SI_USER] user proccess via kill()";
	}

	fprintf(stderr, "Unrecognized signal fetched");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

	int option;
	int argument_count = 0;
	char *arguments[MAX_ARGUMENTS];
	int i;

	int signal;
	siginfo_t signal_info;
	sigset_t signal_set;

	while ((option = getopt(argc, argv, ":s:r:l")) != -1)
		switch (option) {
		case 's': {

			if (argc != 4)
				display_usage(argv);

			for (i = optind - 1; i < optind + 1; i++)
				if (argv[i][0] != '-')
					arguments[argument_count++] = argv[i];
				else {
					optind = i;
					break;
				}

			//send standard signal to another process
			if (kill(atoi(arguments[0]), atoi(arguments[1])) == -1)
				error("kill");
			printf("Standard signal has been sent successfully\n");
			
			exit(EXIT_SUCCESS);
		}
		case 'r': {

			if (argc != 5)
				display_usage(argv);

			for (i = optind - 1; i < optind + 2; i++)
				if (argv[i][0] != '-')
					arguments[argument_count++] = argv[i];
				else {
					optind = i;
					break;
				}

			//send realtime signal to another process
			union sigval value;
			value.sival_int = atoi(arguments[2]);

			if (sigqueue(atoi(arguments[0]), atoi(arguments[1]), value) == -1)
				error("sigqueue");
			printf("Realtime signal has been sent successfully\n");
			
			exit(EXIT_SUCCESS);
		}
		case 'l': {

			//block all signals to become pending
			sigfillset(&signal_set);
			if (sigprocmask(SIG_SETMASK, &signal_set, NULL) == -1)
				error("sigprocmask");

			//synchronously fetch signals except SIGINT and SIGTERM
			printf(
				"Process with PID %ld fetching pending signals:\n\n",
				(long) getpid()
			);
			for (;;) {
				signal = sigwaitinfo(&signal_set, &signal_info);
				if (signal == -1)
					error("sigwaitinfo");
				
				//if SIGINT or SIGTERM is fetched, terminate
				if (signal == SIGINT || signal == SIGTERM) {
					fprintf(
						stdout,
						"\n%s fetched, terminating process\n",
						signal == SIGINT ? "SIGINT" : "SIGTERM"
					);
					
					exit(EXIT_SUCCESS);
				}
				
				//display signal detail
				printf(
					"Signal fetched: %s\n",
					signal_description(signal, signal_info.si_code)
				);
			}

			exit(EXIT_SUCCESS);
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
			display_usage(argv);
		}
}

