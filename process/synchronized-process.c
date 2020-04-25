#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SYNC_SIGNAL SIGUSR1

void error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

static void signal_handler(int signal) {
//for process synchronization purpose
}

int main(int argc, char *argv[]) {

	pid_t child_process;
	sigset_t block_mask, old_mask, empty_mask;
	struct sigaction sa;

	//block the signal
	sigemptyset(&block_mask);
	sigaddset(&block_mask, SYNC_SIGNAL);
	if (sigprocmask(SIG_BLOCK, &block_mask, &old_mask) == -1)
		error("sigprocmask");

	//set the disposition of a signal
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = signal_handler;
	if (sigaction(SYNC_SIGNAL, &sa, NULL) == -1)
		error("sigaction");

	switch (child_process = fork()) {
	case -1:
		error("fork");

	//child process
	case 0:

		//do something and signal parent
		
		if (kill(getppid(), SYNC_SIGNAL) == -1)
			error("kill");
		
		//do other things and exit
		
		_exit(EXIT_SUCCESS);

	//parent process
	default:

		//do something, suspend and wait for child
		
		sigemptyset(&empty_mask);
		if (sigsuspend(&empty_mask) == -1 && errno != EINTR)
			error("sigsuspend");

		/*
		 * parent got signal and can continue, 
		 * signal mask returned to previous state
		 */
		if (sigprocmask(SIG_SETMASK, &old_mask, NULL) == -1)
			error("sigprocmask");
		
		exit(EXIT_SUCCESS);
	}
}

