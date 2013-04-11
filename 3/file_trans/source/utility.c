/* 
 * file: utility.c
 * Author: Joe Shang (ID:1101220731)
 * Desription: Some useful type and program.
 */

#include <unistd.h>
#include <errno.h>

#define BUF_SIZE    1024

ssize_t my_read(int fd, void *buf, size_t size)
{
    ssize_t nread;

    while ((nread = read(fd, buf, size)) == -1 && errno == EINTR) ;

    return nread;
}

ssize_t my_write(int fd, void *buf, size_t size)
{
    size_t total = 0;
	size_t nleft = size;
	ssize_t nwritten;
	void *ptr = buf;

	while (nleft > 0)
	{
        if ((nwritten = write(fd, ptr, nleft)) == -1 && errno == EINTR)
        {
            nwritten = 0;
        }

        if (nwritten == -1)
        {
            return -1;
        }

		ptr += nwritten;
        total += nwritten;
		nleft -= nwritten;
	}

    return total;
}

int copyfile(int to_fd, int from_fd)
{
    char buf[BUF_SIZE];
    int bytes_read;
    int bytes_written;
    int bytes_total = 0;

    for (;;)
    {
        if ((bytes_read = my_read(from_fd, buf, BUF_SIZE)) <= 0)
        {
            break;
        }

        if ((bytes_written = my_write(to_fd, buf, bytes_read)) == -1)
        {
            break;
        }

        bytes_total += bytes_written;
    }
    
    return bytes_total;
}
