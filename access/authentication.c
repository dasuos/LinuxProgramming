#define _GNU_SOURCE
#include <crypt.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <shadow.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int error(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

char *read_username() {
	
	//set username length limit
	long limit = sysconf(_SC_LOGIN_NAME_MAX);
	if (limit == -1)
		limit = 256;

	char *username = malloc(limit);
	if (username == NULL)
		error("malloc");

	//read a username
	printf("Username: ");
	fflush(stdout);
	if (fgets(username, limit, stdin) == NULL)
		exit(EXIT_FAILURE);

	//remove trailing '\n'
	size_t length = strlen(username);
	if (username[length - 1] == '\n')
		username[length - 1] = '\0';

	return username;
}

struct passwd *user_account(char *username) {
	
	//retrieve the password record in /etc/passwd
	struct passwd *record = getpwnam(username);
	if (record == NULL) {
		fprintf(stderr, "User '%s' not found\n", username);
		exit(EXIT_FAILURE);
	}
	return record;
}

struct spwd *shadow_password(char *username){
	
	//retrieve the shadow password record in /etc/shadow
	struct spwd *record = getspnam(username);
	if (record == NULL && errno == EACCES) {
		fprintf(stderr, "No permission to read shadow password file\n");
		exit(EXIT_FAILURE);
	}
	return record;
}

bool authenticated(struct passwd *user_account, struct spwd *shadow_password) {
	
	if (shadow_password != NULL) {

		//no password is required
		if (strcmp(shadow_password->sp_pwdp, "*") == 0)
			return true;

		//the account is locked
		if (strcmp(shadow_password->sp_pwdp, "!") == 0 || strcmp(shadow_password->sp_pwdp, "!!") == 0) {
			fprintf(stderr, "The account is locked\n");
			exit(EXIT_FAILURE);
		}

		user_account->pw_passwd = shadow_password->sp_pwdp;
	}

	/*
	 * prompt the user for a password and erase cleartext version 
	 * immediately to avoid reading the password from the swap file 
	 * or /dev/mem 
	 */
	char *password = getpass("Password: ");
	char *encrypted_password = crypt(password, user_account->pw_passwd);
	for (char *p = password; *p != '\0'; )
		*p++ = '\0';

	if (encrypted_password == NULL)
		error("crypt");

	return strcmp(encrypted_password, user_account->pw_passwd) == 0;
}

int main(int argc, char *argv[]) {
	
	char *username = read_username();

	if (authenticated(user_account(username), shadow_password(username)))
		printf("You have been successfully authenticated\n");
	else
		printf("Incorrect password\n");

	return EXIT_SUCCESS;
}

