#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

void my_func(int);
void kill_s1(int);
void kill_s2(int);

int pipefd[2];
pid_t s1, s2;

int main(void)
{
	signal(SIGINT, my_func);
	char buf[128]; //send buf
	pipe(pipefd);
	s1 = fork();
	if (s1 == 0)
	{
		//son 1
		signal(SIGINT, SIG_IGN);
		signal(SIGUSR1, kill_s1);

		while (1)
		{
			close(pipefd[1]);
			read(pipefd[0], buf, 128);
			cout << buf;
		}
	}
	else
	{
		//parent
		s2 = fork();
		if (s2 == 0)
		{
			//son 2
			signal(SIGINT, SIG_IGN);
			signal(SIGUSR2, kill_s2);

			while (1)
			{
				close(pipefd[1]);
				read(pipefd[0], buf, 128);
				cout << buf;
			}
		}
		else
		{
			int x = 1;
			while (x < 10)
			{
				close(pipefd[0]);
				sprintf(buf, "I send you %d times\n", x);
				write(pipefd[1], buf, 128);
				x++;
				sleep(1);
			}
			waitpid(s1, NULL, 0);
			waitpid(s2, NULL, 0);
			close(pipefd[1]);
			close(pipefd[0]);
			cout << "Parent Process is Killed!\n";
			exit(0);
		}
	}
}

void my_func(int)
{
	signal(SIGINT, SIG_DFL);
	kill(s1, SIGUSR1);
	kill(s2, SIGUSR2);
}

void kill_s1(int)
{
	cout << "Child Process l is Killed by Parent!\n";
	exit(0);
}

void kill_s2(int)
{
	cout << "Child Process 2 is Killed by Parent!\n";
	exit(0);
}