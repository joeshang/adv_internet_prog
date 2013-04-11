/*
 * File: trans_client.c
 * Author: Joe Shang (ID:1101220731)
 * Description: The client program of transporting files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "utility.h"

#define BUF_SIZE        1024
#define OPEN_FLAGS      (O_WRONLY | O_CREAT | O_TRUNC)
#define OPEN_PERMS      (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define DEF_RECV_DIR    "../recv_files"    

int create_recv_file(char *req_file_path, char *recv_dir);

int main(int argc, char *argv[])
{
	int status = 0;
	int recv_fd;
    char recv_buf[BUF_SIZE];
    int req_file_len = 0;
	int connect_socket;
	char *address;
	char *port;
	struct sockaddr_in serv_addr;

	if (argc != 3 && argc != 4)
	{
		fprintf(stderr, "Usage: %s address:port req_file_name [recv_directory]\n", argv[0]);
		exit(1);
	}

	if ((address = strtok(argv[1], ":")) == NULL)
	{
		fprintf(stderr, "wrong input format of address:port\n");
		exit(1);
	}
	port = strtok(NULL, ":");


	/* Set up connection */
	/* .1 Set up socket */
	if ((connect_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error");
		exit(1);
	}

	/* .2 Initialize address structure */
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port  = htons(atoi(port));
	if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0)
	{
		char buf[200];
		snprintf(buf, sizeof(buf), "inet_pton error for %s", "");
		perror(buf);
		exit(1);
	}

	/* .3 Connect to server */
	if (connect(connect_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("connect error");
		exit(1);
	}
	printf("client connect to server %s:%s successfully\n", address, port);

	/* Send the requesting file name */
	my_write(connect_socket, argv[2], strlen(argv[2]) + 1);

	/* Receive request status */
	my_read(connect_socket, (void *)&status, sizeof(int));

    if (status == REQ_OK)
    {
        /* Receive the requesting file size */
        my_read(connect_socket, (void *)&req_file_len, sizeof(int));
        printf("begin to downloading file %s [size: %d bytes] ...\n", argv[2], req_file_len);

        /* Requested valid, create corresponding file in client */
        if (argc == 3)	/* No receive directory, use requesting file name directly */
        {
            recv_fd = create_recv_file(argv[2], NULL);
        }
        else if (argc == 4) /* Construct receive path with requesting file name and directory */
        {
            recv_fd = create_recv_file(argv[2], argv[3]);
        }

        if (recv_fd != -1) /* Create receive file successfully */
        {
            /* Receive file contents and write these contents info file */
            int recv_len;
            int nleft = req_file_len;

            while (nleft > 0)
            {
                if ((recv_len = my_read(connect_socket, recv_buf, BUF_SIZE)) <= 0)
                {
                    break;
                }

                if (my_write(recv_fd, recv_buf, recv_len) == -1)
                {
                    perror("write error");
                    exit(1);
                }

                nleft -= recv_len;
                int percent = (double)(req_file_len - nleft) / (double)req_file_len * 100;
                printf("[%3d%%] receive %d bytes\n", percent, recv_len);
            }

            close(recv_fd);
        }
    }
    else if (status == REQ_INVALID_NAME)
    {
        fprintf(stderr, "invalid file name\n");
    }
    else
    {
        fprintf(stderr, "unknown server error\n");
    }

	/* Close connection */
	close(connect_socket);

	return 0;
}

int create_recv_file(char *req_file_path, char *recv_dir)
{
	char *token = NULL;
	char *req_file_name = NULL;
	char *recv_file_path = NULL;
	int recv_file_path_len = 0;
	int recv_fd;
	int errbak;

	/* Handle req_file_name with directory */
	if ((req_file_name = strtok(req_file_path, "/")) != NULL)
	{
		while((token = strtok(NULL, "/")) != NULL)
		{
			req_file_name = token;
		}
	}
	else
	{
		req_file_name = req_file_path;
	}

	/* Create the receive file in client */
	if (recv_dir == NULL)	/* No receive directory, use the default receive directory */
	{
        recv_dir = DEF_RECV_DIR;
	}

	/* Construct receive path with requesting file name and directory */
    recv_file_path_len = strlen(recv_dir) + 1 + strlen(req_file_name) + 1;
    recv_file_path = (char *)malloc(sizeof(char) * recv_file_path_len);
    memset(recv_file_path, 0, recv_file_path_len);	
    strcat(recv_file_path, recv_dir);
    strcat(recv_file_path, "/");
    strcat(recv_file_path, req_file_name);

    if ((recv_fd = open(recv_file_path, OPEN_FLAGS, OPEN_PERMS)) == -1)
    {
        errbak = errno;
        free(recv_file_path);
        errno = errbak;
        perror("Failed to open file");
    }
    
	return recv_fd;
}
