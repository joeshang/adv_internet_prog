/********************************************************************************
 * FileName: signal.c
 * Author: Joe Shang(1101220731)
 * Description: The second program of internet programming's homework one.
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#define REPEAT_TIMES	5

void sig_handler(int signum);

int main(int argc, char* argv[])
{
	pid_t child_pid;

	/* Bind signal handler */
	signal(SIGUSR1, sig_handler);
	signal(SIGUSR2, sig_handler);

	child_pid = fork();
	if (child_pid < 0)
	{
		perror("Failed to fork");
		exit(1);
	}
	else if (child_pid == 0)	/* Child process */
	{
		int count = REPEAT_TIMES;
		while (count-- > 0)
		{
			kill(getppid(), SIGUSR1);
			sleep(1);
		}

		kill(getppid(), SIGUSR2);
		printf("Finished to send signal SIGUSR2, child process %d exit.\n", getpid());
	}
	else	/* Parent process */
	{
		/* Wait for child process to make sure child process run befor parent */
		if (wait(NULL) < 0)
		{
			perror("Failed to wait");
		}
		printf("Finished to handle signal SIGUSR2, parent process %d exit\n", getpid());
	}

	return 0;
}

void sig_handler(int signum)
{
	if (signum == SIGUSR1)
	{
		/* Print current system time */
		time_t curr_time;
		time(&curr_time);
		printf("Parent process %d catch signal SIGUSR1, current time is %s", getpid(), ctime(&curr_time));
	}
	else if (signum == SIGUSR2)
	{
		printf("Parent process %d catch signal SIGUSR2\n", getpid());
	}
}
