#define _GNU_SOURCE
#include <errno.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

char *signal_description(int signal, int code) {

	if (signal == SIGBUS)
		return "[SIGBUS] memory access errors";
	else if (signal == SIGCHLD)
		return "[SGCHLD] child process terminates";
	else if (signal == SIGFPE)
		return "[SIGFPE] arithmetic error occurs";
	else if (signal == SIGILL)
		return "[SIGILL] illegal machine-language instruction";
	else if (signal == SIGSEGV)
		return "[SIGSEGV] invalid memory reference";
	else if (signal == SIGTRAP)
		return "[SIGTRAP] debugger breakpoints";
	else if (code == SI_ASYNCIO)
		return "[SI_ASYNCIO] completion of an asynchronous I/O";
	else if (code == SI_KERNEL)
		return "[SI_KERNEL] sent by the kernel";
	else if (code == SI_MESGQ)
		return "[SI_MESGQ] message arrival on POSIX message queue";
	else if (code == SI_QUEUE)
		return "[SI_QUEUE] realtime signal";
	else if (code == SI_SIGIO)
		return "[SI_SIGIO] SIGIO signal";
	else if (code == SI_TIMER)
		return "[SI_TIMER] expiration of a POSIX timer";
	else if (code == SI_TKILL)
		return "[SI_TKILL] user process via tkill() or tgkill()";
	else if (code == SI_USER)
		return "[SI_USER] user proccess via kill()";
	
	fprintf(stderr, "Unrecognized signal fetched");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {

	int option;
	int argument_count = 0;
	char *arguments[3];
	int i;

	int signal;
	siginfo_t signal_info;
	sigset_t signal_set;

	while ((option = getopt(argc, argv, ":s:r:l")) != -1)
		switch (option) {
		case 's': {

			for (i = optind - 1; i < optind + 1; i++)
				if (argv[i][0] != '-')
					arguments[argument_count++] = argv[i];
				else {
					optind = i;
					break;
				}

			//send standard process to another process
			if (kill(atoi(arguments[0]), atoi(arguments[1])) == -1)
				error("kill");
			printf("Standard signal has been sent successfully\n");
			
			exit(EXIT_SUCCESS);
		}
		case 'r': {
			
			for (i = optind - 1; i < optind + 2; i++)
				if (argv[i][0] != '-')
					arguments[argument_count++] = argv[i];
				else {
					optind = i;
					break;
				}

			//send realtime process to another process
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
			fprintf(
				stderr,
				"Usage: %s -l | -s pid sig | -r pid sig value\n",
				argv[0]
			);
			exit(EXIT_FAILURE);
		}
}

