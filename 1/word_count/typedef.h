/*
 * File:    typedef.h
 * Author:	Joe Shang (ID:1101220731)
 * Brief:   common types definition.
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

typedef enum _Ret
{
	RET_OK,
	RET_OOM,
	RET_STOP,
	RET_INVALID_PARAMS,
	RET_FAIL
}Ret;

typedef void (*DataDestroyFunc)(void* ctx, void* data);
typedef int  (*DataCompareFunc)(void* ctx, void* data);
typedef Ret  (*DataVisitFunc)(void* ctx, void* data);
typedef Ret	 (*SortFunc)(void** array, size_t nr, DataCompareFunc cmp);

#ifdef __cplusplus
#define DECLS_BEGIN extern "C" {
#define DECLS_END   }
#else
#define DECLS_BEGIN
#define DECLS_END
#endif/*__cplusplus*/

#define return_if_fail(p) if(!(p)) \
	{printf("%s:%d Warning: "#p" failed.\n", \
		__func__, __LINE__); return;}
#define return_val_if_fail(p, ret) if(!(p)) \
	{printf("%s:%d Warning: "#p" failed.\n",\
	__func__, __LINE__); return (ret);}

#define SAFE_FREE(p) if (p != NULL) { free(p); p = NULL; }

#endif/*TYPEDEF_H*/

