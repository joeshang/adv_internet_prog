#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUF_SIZE        1024
#define MAX_NAME_LEN    20
#define IDENTIER_LEN    10
#define IDENTIER        " said: "

int main(int argc, char *argv[])
{
    int connect_socket;
    struct sockaddr_in serv_addr;
    fd_set read_set;
    char input_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];
    char send_buf[MAX_NAME_LEN + IDENTIER_LEN + BUF_SIZE];
    
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s address port name\n", argv[0]);
        exit(1);
    }
    
    if (strlen(argv[3]) > MAX_NAME_LEN)
    {
        fprintf(stderr, "Too long user name, should less than 20 characters\n");
        exit(1);
    }

    /* 1. create socket */
    if ((connect_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        exit(1);
    }

    /* 2. initialize address structure */
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    /* 3. connect to the server */
    if (connect(connect_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        perror("connect error");
        exit(1);
    }

    printf("Enter into chat room:\n");

    FD_ZERO(&read_set);

    for (;;)
    {
        FD_SET(fileno(stdin), &read_set);
        FD_SET(connect_socket, &read_set);
        select(connect_socket + 1, &read_set, NULL, NULL, NULL);

        if (FD_ISSET(fileno(stdin), &read_set))
        {
            if (fgets(input_buf, BUF_SIZE, stdin) == NULL)
            {
                shutdown(connect_socket, SHUT_WR);
                exit(0);
            }

            strcpy(send_buf, argv[3]);
            strcat(send_buf, IDENTIER);
            strcat(send_buf, input_buf);

            fputs(send_buf, stdout);

            if (send(connect_socket, send_buf, strlen(send_buf) + 1, 0) == -1)
            {
                perror("send error");
                exit(1);
            }
        }

        if (FD_ISSET(connect_socket, &read_set))
        {
            if (recv(connect_socket, recv_buf, BUF_SIZE, 0) == 0)
            {
                printf("server down\n");

                exit(0);
            }

            fputs(recv_buf, stdout);
        }
    }

    return 0;
}
