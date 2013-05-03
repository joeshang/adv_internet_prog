/**
 * File: chat_server.c
 * Author: Joe Shang (ID: 1101220731)
 * Brief: The server program of chat room.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG     10
#define BUF_SIZE    1024

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

int main(int argc, char *argv[])
{
    int listen_socket;
    int connect_socket;

    struct sockaddr_in server_addr;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;
    socklen_t server_raddr_len;
    struct sockaddr_in server_raddr;

    pid_t pid;

    fd_set set_bak;
    fd_set read_set;
    fd_set cli_set_bak;
    fd_set cli_read_set;
    int max_fd;
    char msg_buf[BUF_SIZE];

    int i;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(1);
    }

    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        exit(1);
    }

    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("listen error");
        exit(1);
    }

    if (listen(listen_socket, BACKLOG) == -1)
    {
        perror("listen error");
        exit(1);
    }

    server_raddr_len = sizeof(server_raddr);
    if (getsockname(listen_socket, (struct sockaddr *)&server_raddr, &server_raddr_len) == -1)
    {
        perror("getsockname error");
        exit(1);
    }
    char disp_addr_buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server_raddr.sin_addr, disp_addr_buf, INET_ADDRSTRLEN);
    printf("server is listenning at %s:%d\n", disp_addr_buf, ntohs(server_raddr.sin_port));

    int pfd[2];
    if (pipe(pfd) < 0)
    {
        perror("pipe error");
        exit(1);
    }

    int cli_pfd[FD_SETSIZE][2];
    for (i = 0; i < FD_SETSIZE; i++)
    {
        cli_pfd[i][0] = -1;
        cli_pfd[i][1] = -1;
    }

    signal(SIGCHLD, sig_handler);

    FD_ZERO(&set_bak);
    FD_SET(listen_socket, &set_bak);
    FD_SET(pfd[0], &set_bak);
    max_fd = (listen_socket >= pfd[0]) ? listen_socket : pfd[0];

    for (;;)
    {
        read_set = set_bak;

        if (select(max_fd + 1, &read_set, NULL, NULL, NULL) == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("select error");
                exit(1);
            }
        }

        if (FD_ISSET(listen_socket, &read_set))
        {
            client_addr_len = sizeof(client_addr);
            if ((connect_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1)
            {
                perror("accept error");
                exit(1);
            }

            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (cli_pfd[i][0] == -1 || cli_pfd[i][1] == -1)
                {
                    break;
                }
            }

            if (pipe(cli_pfd[i]) < 0)
            {
                perror("pipe error");
                exit(1);
            }

            if ((pid = fork()) < 0)
            {
                perror("fork error");
                exit(1);
            }
            else if (pid > 0)
            {
                close(connect_socket);
                close(cli_pfd[i][0]);
            }
            else
            {
                close(listen_socket);
                close(cli_pfd[i][1]);

                FD_ZERO(&cli_set_bak);
                FD_SET(connect_socket, &cli_set_bak);
                FD_SET(cli_pfd[i][0], &cli_set_bak);
                max_fd = (connect_socket >= cli_pfd[i][0]) ? connect_socket : cli_pfd[i][0];

                for (;;)
                {
                    cli_read_set = cli_set_bak;

                    select(max_fd + 1, &cli_read_set, NULL, NULL, NULL);

                    if (FD_ISSET(connect_socket, &cli_read_set))
                    {
                        if (recv(connect_socket, msg_buf, BUF_SIZE, 0) <= 0)
                        {
                            close(connect_socket);
                            printf("client quit\n");
                            snprintf(msg_buf, BUF_SIZE, "#%d", i);
                            write(pfd[1], msg_buf, strlen(msg_buf) + 1);
                            break;
                        }
                        else
                        {
                            printf("socket: %s\n", msg_buf);
                            write(pfd[1], msg_buf, strlen(msg_buf) + 1);
                        }
                    }

                    if (FD_ISSET(cli_pfd[i][0], &cli_read_set))
                    {
                        read(cli_pfd[i][0], msg_buf, BUF_SIZE);
            //            printf("pipe: %s\n", msg_buf);
                        send(connect_socket, msg_buf, strlen(msg_buf) + 1, 0);
                    }
                }

                close(connect_socket);
                exit(0);
            }
        }

        if (FD_ISSET(pfd[0], &read_set))
        {
            printf("pipe data came, ");
            read(pfd[0], msg_buf, BUF_SIZE);
            printf("get pipe data: %s\n", msg_buf);

            int j;

            if (msg_buf[0] != '#')
            {
                for (j = 0; j < FD_SETSIZE; j++)
                {
                    if (cli_pfd[j][1] != -1)
                    {
                        write(cli_pfd[j][1], msg_buf, strlen(msg_buf) + 1);
                    }
                }
            }
            else
            {
                j = atoi(msg_buf + 1);

                printf("child process's pipe %d close\n", j);

                close(cli_pfd[j][0]);
                close(cli_pfd[j][1]);

                cli_pfd[j][0] = -1;
                cli_pfd[j][1] = -1;
            }

        }
    }

    close(listen_socket);

    return 0;
}
