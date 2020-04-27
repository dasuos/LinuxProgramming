#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile int live_children = 0;

void error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

void sigchld_handler(int signal) {
	
	int status, saved_errno;
	pid_t child_process;

	char const message[] = "Child process exitted\n";

	//save errno to avoid race condition
        saved_errno = errno;
	
	/*
	 * reap zombie children in iteration in case of multiple signals 
	 * that are not queued
	 */
	while ((child_process = waitpid(-1, &status, WNOHANG)) > 0) {
		write(STDOUT_FILENO, message, sizeof(message));
		live_children--;
	}
	
	if (child_process == -1 && errno != ECHILD)
		error("waitpid");

	errno = saved_errno;
}

int main(int argc, char *argv[]) {

	sigset_t blocked_mask, empty_mask;
	struct sigaction sa;

	if (argc < 2 || strcmp(argv[1], "--help") == 0) {
		fprintf(
			stderr,
			"Usage: %s child-duration...\n",
			argv[0]
		);
		exit(EXIT_FAILURE);
	}

	live_children = argc - 1;

	//set the disposition of SIGCHLD signal
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sigchld_handler;
	if (sigaction(SIGCHLD, &sa, NULL) == -1)
		error("sigaction");

	//block SIGCHLD until the sigsuspend()
	sigemptyset(&blocked_mask);
	sigaddset(&blocked_mask, SIGCHLD);
	if (sigprocmask(SIG_SETMASK, &blocked_mask, NULL) == -1)
		error("sigprocmask");

	for (int i = 1; i < argc; i++) {
		switch (fork()) {
		case -1:
			error("fork");
			
		//child process
		case 0:
			//sleep for given seconds
			sleep(atoi(argv[i]));
			_exit(EXIT_SUCCESS);
		default:
			break;
		}
	}

	//wait until all child processes are dead
	sigemptyset(&empty_mask);
	while (live_children > 0)
		if (sigsuspend(&empty_mask) == -1 && errno != EINTR)
			error("sigsuspend");

	exit(EXIT_SUCCESS);
}

