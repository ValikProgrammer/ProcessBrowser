#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>

// Test: SIGTERM terminates a child process
static int test_sigterm(void)
{
	pid_t pid = fork();

	if (pid < 0) {
		fprintf(stderr, "FAIL: fork() failed\n");
		return 1;
	}

	if (pid == 0) {
		// Child: sleep indefinitely
		while (1) {
			sleep(1);
		}
		exit(0);
	}

	// Parent: send SIGTERM to child
	sleep(1); // Give child time to start
	if (kill(pid, SIGTERM) != 0) {
		fprintf(stderr, "FAIL: kill(SIGTERM) failed\n");
		return 1;
	}

	int status;
	waitpid(pid, &status, 0);

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM) {
		printf("PASS: SIGTERM terminated child\n");
		return 0;
	} else {
		fprintf(stderr, "FAIL: child did not terminate with SIGTERM\n");
		return 1;
	}
}

// Test: SIGKILL terminates a child process
static int test_sigkill(void)
{
	pid_t pid = fork();

	if (pid < 0) {
		fprintf(stderr, "FAIL: fork() failed\n");
		return 1;
	}

	if (pid == 0) {
		// Child: ignore SIGTERM and sleep
		signal(SIGTERM, SIG_IGN);
		while (1) {
			sleep(1);
		}
		exit(0);
	}

	// Parent: send SIGKILL to child
	sleep(1); // Give child time to start
	if (kill(pid, SIGKILL) != 0) {
		fprintf(stderr, "FAIL: kill(SIGKILL) failed\n");
		return 1;
	}

	int status;
	waitpid(pid, &status, 0);

	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGKILL) {
		printf("PASS: SIGKILL terminated child\n");
		return 0;
	} else {
		fprintf(stderr, "FAIL: child did not terminate with SIGKILL\n");
		return 1;
	}
}

int main(void)
{
	int failures = 0;

	printf("Running integration tests for process killing...\n");

	failures += test_sigterm();
	failures += test_sigkill();

	if (failures == 0) {
		printf("All kill tests passed.\n");
		return 0;
	} else {
		fprintf(stderr, "%d test(s) failed.\n", failures);
		return 1;
	}
}
