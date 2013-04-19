/*
 * File: chat_server.c
 * Author: Joe Shang (ID: 1101220731)
 * Brief: The server program(thread implement version) of chat room
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define ROOM_MAX        100
#define BACKLOG         10
#define BUF_SIZE        1024 

int max_index;
int client[ROOM_MAX];

void *client_handler(void *arg)
{
    int i;
    char msg_buf[BUF_SIZE];
    int connect_socket = (int)arg;
    pthread_mutex_t lock;

    pthread_mutex_init(&lock, NULL);

    for (;;)
    {
        if (recv(connect_socket, msg_buf, BUF_SIZE, 0) <= 0)
        {
            close(connect_socket);

            pthread_mutex_lock(&lock);
            for (i = 0; i < max_index; i++)
            {
                if (client[i] == connect_socket)
                {
                    client[i] = -1;
                    break;
                }
            }
            pthread_mutex_unlock(&lock);

            printf("%d connection finished\n", connect_socket);

            pthread_mutex_destroy(&lock);
            pthread_exit(NULL);
        }
        else
        {
            printf("receive message: %s\n", msg_buf);

            pthread_mutex_lock(&lock);
            for (i = 0; i <= max_index; i++)
            {
                if (client[i] >= 0 && client[i] != connect_socket)
                {
                    send(client[i], msg_buf, strlen(msg_buf) + 1, 0);
                }
            }
            pthread_mutex_unlock(&lock);
        }
    }
}

int main(int argc, char *argv[])
{
    int listen_socket;
    int connect_socket;
    
    struct sockaddr_in serv_addr;
    socklen_t client_addr_len;
    struct sockaddr_in client_addr;
    socklen_t serv_disp_addr_len;
    struct sockaddr_in serv_disp_addr;

    pthread_t ntid;
    pthread_mutex_t lock;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(1);
    }

    /* 1. create socket for listenning */
    if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        exit(1);
    }

    /* 2. configure address/port and set for reuse */ 
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    /* 3. bind socket */
    if (bind(listen_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("bind error");
        exit(1);
    }

    /* 4. listen for address+port */
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
    char serv_disp_addr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &serv_disp_addr.sin_addr, serv_disp_addr_str, INET_ADDRSTRLEN);
    printf("server is listenning at %s:%d\n", serv_disp_addr_str, ntohs(serv_disp_addr.sin_port));

    int i;
    for (i = 0; i < ROOM_MAX; i++)
    {
        client[i] = -1;
    }
    max_index = -1;

    pthread_mutex_init(&lock, NULL);

    for (;;)
    {
        /* accept for client's connections */
        client_addr_len = sizeof(client_addr);
        if ((connect_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1)
        {
            perror("accept error");
            exit(1);
        }
        printf("%d connection begin\n", connect_socket);

        pthread_mutex_lock(&lock);
        for (i = 0; i < ROOM_MAX; i++)
        {
            if (client[i] < 0)
            {
                client[i] = connect_socket;
                break;
            }
        }

        if (i > max_index)
        {
            max_index = i;
        }
        pthread_mutex_unlock(&lock);
        
        pthread_create(&ntid, NULL, client_handler, (void *)connect_socket);
    }

    return 0;
}
