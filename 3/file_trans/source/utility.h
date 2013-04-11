/* 
 * file: utility.h
 * Author: Joe Shang (ID:1101220731)
 * Desription: Some useful type and program.
 */

#ifndef _UTILITY_H_
#define _UTILITY_H_

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    REQ_OK = 1,
    REQ_INVALID_NAME
};

ssize_t my_read(int fd, void *buf, size_t size);
ssize_t my_write(int fd, void *buf, size_t size);

int copyfile(int to_fd, int from_fd);

#ifdef __cplusplus
}
#endif

#endif
