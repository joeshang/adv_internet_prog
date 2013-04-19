#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BACKLOG     10
#define BUF_SIZE    1024

int main(int argc, char *argv[])
{
    int connect_socket;
    int listen_socket;
    struct sockaddr_in serv_addr;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;
    socklen_t serv_disp_addr_len;
    struct sockaddr_in serv_disp_addr;

    fd_set read_set;
    fd_set read_bak_set;
    int ready_num;

    int client_index;
    int send_index;
    int max_fd;
    int max_index;
    int client[FD_SETSIZE];

    char msg_buf[BUF_SIZE];

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(1);
    }

    /* 1. create socket and set socket option for reusing address and port */
    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        exit(1);
    }

    /* 2. initialize address structure */
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    /* 3. bind socket to address:port */
    if (bind(listen_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("bind error");
        exit(1);
    }

    /* 4. listen at socket */
    if (listen(listen_socket, BACKLOG) == -1)
    {
        perror("listen error");
        exit(1);
    }

    serv_disp_addr_len = sizeof(serv_disp_addr);
    if (getsockname(listen_socket, (struct sockaddr *)&serv_disp_addr, &serv_disp_addr_len) == -1)
    {
        perror("getsockname error");
        exit(1);
    }
    char serv_disp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &serv_disp_addr.sin_addr, serv_disp, INET_ADDRSTRLEN);
    printf("server is listenning at %s:%d\n", serv_disp, ntohs(serv_disp_addr.sin_port));

    for (client_index = 0; client_index < FD_SETSIZE; client_index++)
    {
        client[client_index] = -1;
    }
    max_index = -1;

    /* 5. set up select set for client connection */
    FD_ZERO(&read_set);
    FD_ZERO(&read_bak_set);
    FD_SET(listen_socket, &read_bak_set);
    max_fd = listen_socket;

    for (;;)
    {
        read_set = read_bak_set;

        ready_num = select(max_fd + 1, &read_set, NULL, NULL, NULL);

        /* new client connection */
        if (FD_ISSET(listen_socket, &read_set))
        {
            client_addr_len = sizeof(client_addr);
            if ((connect_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1)
            {
                perror("accept error");
                exit(1);
            }

            printf("%d connection begin\n", connect_socket);

            for (client_index = 0; client_index < FD_SETSIZE; client_index++)
            {
                if (client[client_index] == -1)
                {
                    client[client_index] = connect_socket;
                    break;
                }
            }

            if (client_index == FD_SETSIZE)
            {
                printf("too many client\n");
                exit(1);
            }

            if (client_index > max_index)
            {
                max_index = client_index;
            }

            if (connect_socket > max_fd)
            {
                max_fd = connect_socket;
            }

            FD_SET(connect_socket, &read_bak_set);

            /* no read event, go to select directly */
            if (--ready_num <= 0)
            {
                continue;
            }
        }

        for (client_index = 0; client_index <= max_index; client_index++)
        {
            if (client[client_index] < 0)
            {
                continue;
            }

            /* client send message to server */
            if (FD_ISSET(client[client_index], &read_set))
            {
                if (recv(client[client_index], msg_buf, BUF_SIZE, 0) <= 0)
                {
                    /* client close */
                    close(client[client_index]);
                    FD_CLR(client[client_index], &read_set);

                    printf("%d connection finished\n", client[client_index]);

                    client[client_index] = -1;
                }
                else
                {
                    printf("receive message: %s\n", msg_buf);
                    for (send_index = 0; send_index <= max_index; send_index++)
                    {
                        if (client[send_index] > 0 && client[send_index] != client[client_index])
                        {
                            send(client[send_index], msg_buf, strlen(msg_buf) + 1, 0);
                        }
                    }

                    /* no client's msg, exit loop */
                    if (--ready_num <= 0)
                    {
                        break;
                    }
                }
            }
        }
    }

    return 0;
}
