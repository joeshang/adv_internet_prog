/* 
 * file: file_server.c
 * Author: Joe Shang (ID:1101220731)
 * Desription: The multi-process file server program
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

#include "utility.h"

#define BACKLOG			10
#define BUF_SIZE		1024
#define OPEN_FLAGS		O_RDONLY
#define SERV_PORT		5000
#define SERV_DB_PATH	"../server_db/"

void sig_handler(int signum);

int main(int argc, char *argv[])
{
    int on;
    int pid;
	int req_fd;
    int req_file_status;
	int connect_socket;
	int listen_socket;
    int req_file_len;
	int req_file_path_len;
	char *req_file_path;
	char read_buf[BUF_SIZE];
	char client_addr_buf[INET_ADDRSTRLEN];
	socklen_t curr_len;
	socklen_t client_len;
	struct sockaddr_in serv_addr;
	struct sockaddr_in client_addr;
	struct sockaddr_in curr_addr;

	/* Set up server socket and wait for connection */
	/* .1 Set up socket and set address/port reuse */
	if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket error");
		exit(1);
	}

    on = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1)
    {
        perror("setsockopt error");
    }
    
#ifdef  SO_REUSEPORT
    on = 1;
    if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on)) == -1)
    {
        perror("setsockopt error");
    }
#endif

	/* .2 Initialize address structure */
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_PORT);
	
	/* .3 Bind the target port */
	if (bind(listen_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
	{
		perror("bind error");
		exit(1);
	}

	/* .4 Listen for connection */
	if (listen(listen_socket, BACKLOG) == -1)
	{
		perror("listen error");
		exit(1);
	}

	curr_len = sizeof(curr_addr);
	if (getsockname(listen_socket, (struct sockaddr *)&curr_addr, &curr_len) == -1)
	{
		perror("getsockname error");
		exit(1);
	}
	printf("server is listening at %s:%d\n", inet_ntoa(curr_addr.sin_addr), ntohs(curr_addr.sin_port));

    signal(SIGCHLD, sig_handler);

	for (;;)
	{
		client_len = sizeof(client_addr);
		if ((connect_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &client_len)) == -1)
		{
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("accept error");
                exit(1);
            }
		}	

		/* Multi-process/multi-thread handling */
        pid = fork();
        if (pid == 0)
        {
            close(listen_socket);

            inet_ntop(AF_INET, &client_addr.sin_addr, client_addr_buf, INET_ADDRSTRLEN);
            printf("\nserver child process %d accept from client %s\n", getpid(), client_addr_buf);

            /* Get file name from client and open corresponding file in server */
            my_read(connect_socket, read_buf, BUF_SIZE);
            printf("requesting file name: %s\n", read_buf);

            req_file_path_len = strlen(SERV_DB_PATH) + strlen(read_buf) + 1;
            req_file_path = (char *)malloc(sizeof(char) * req_file_path_len);
            strcat(req_file_path, SERV_DB_PATH);
            strncat(req_file_path, read_buf, strlen(read_buf));

            if ((req_fd = open(req_file_path, OPEN_FLAGS)) == -1)   /* File doesn't existed */
            {
                printf("invalid requesting file name %s\n", req_file_path);

                req_file_status = REQ_INVALID_NAME;
                my_write(connect_socket, (void *)&req_file_status, sizeof(int));
            }
            else    /* File existed */
            {
                req_file_status = REQ_OK;
                my_write(connect_socket, (void *)&req_file_status, sizeof(int));

                /* Send requesting file size */
                req_file_len = lseek(req_fd, 0, SEEK_END);
                lseek(req_fd, 0, SEEK_SET);
                my_write(connect_socket, (void *)&req_file_len, sizeof(int));

                printf("begin to send file %s [size: %d bytes] ...\n", req_file_path, req_file_len);

                /* Send requesting file contents */
                copyfile(connect_socket, req_fd);

                close(req_fd);
            }

            /* chlid process exited */
            close(connect_socket);
            exit(0);
        }

		close(connect_socket);
	}

	close(listen_socket);

	return 0;
}

void sig_handler(int signum)
{
    int stat;
    pid_t child_pid;

    switch (signum)
    {
        case SIGCHLD:
            while ((child_pid = waitpid(-1, &stat, WNOHANG)) > 0)
            {
                printf("server child process %d finished\n", child_pid);
            }
            break;
        default:
            break;
    }
}
