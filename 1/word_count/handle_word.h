/*
 * File:	handle_word.c
 * Author:	Joe Shang (ID:1101220731)
 * Brief:	Word division module
 */

#ifndef _HANDLE_WORD_
#define _HANDLE_WORD_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define MAX_WORD_LEN	256

typedef void (*OneWordFinishHandler)(void *ctx, char *string);

void divide_word(FILE *file, OneWordFinishHandler handler, void *ctx);

#ifdef __cplusplus
}
#endif

#endif
