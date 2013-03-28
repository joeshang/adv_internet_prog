/*
 * File:    darray.c
 * Author:  Joe Shang (ID:1101220731)
 * Brief:   Dynamic array implementation.
 */

#include <stdlib.h>
#include "darray.h"

struct _DArray
{
	void** data;
	size_t size;
	size_t alloc_size;

	void* data_destroy_ctx;
	DataDestroyFunc data_destroy;
};

static void darray_destroy_data(DArray* thiz, void* data)
{
	if(thiz->data_destroy != NULL)
	{
		thiz->data_destroy(thiz->data_destroy_ctx, data);
	}

	return;
}

DArray* darray_create(DataDestroyFunc data_destroy, void* ctx)
{
	DArray* thiz = malloc(sizeof(DArray));

	if(thiz != NULL)
	{
		thiz->data  = NULL;
		thiz->size  = 0;
		thiz->alloc_size = 0;
		thiz->data_destroy = data_destroy;
		thiz->data_destroy_ctx = ctx;
	}

	return thiz;
}

#define MIN_PRE_ALLOCATE_NR 10
static Ret darray_expand(DArray* thiz, size_t need)
{
	return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS); 

	if((thiz->size + need) > thiz->alloc_size)
	{
		size_t alloc_size = thiz->alloc_size + (thiz->alloc_size>>1) + MIN_PRE_ALLOCATE_NR;

		void** data = (void**)realloc(thiz->data, sizeof(void*) * alloc_size);
		if(data != NULL)
		{
			thiz->data = data;
			thiz->alloc_size = alloc_size;
		}
	}

	return ((thiz->size + need) <= thiz->alloc_size) ? RET_OK : RET_FAIL;
}

static Ret darray_shrink(DArray* thiz)
{
	return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS); 

	if((thiz->size < (thiz->alloc_size >> 1)) && (thiz->alloc_size > MIN_PRE_ALLOCATE_NR))
	{
		size_t alloc_size = thiz->size + (thiz->size >> 1);

		void** data = (void**)realloc(thiz->data, sizeof(void*) * alloc_size);
		if(data != NULL)
		{
			thiz->data = data;
			thiz->alloc_size = alloc_size;
		}
	}

	return RET_OK;
}

Ret darray_insert(DArray* thiz, size_t index, void* data)
{
	Ret ret = RET_OOM;
	size_t cursor = index;
	return_val_if_fail(thiz != NULL, RET_INVALID_PARAMS); 

	cursor = cursor < thiz->size ? cursor : thiz->size;

	if(darray_expand(thiz, 1) == RET_OK)
	{
		size_t i = 0;
		for(i = thiz->size; i > cursor; i--)
		{
			thiz->data[i] = thiz->data[i-1];
		}

		thiz->data[cursor] = data;
		thiz->size++;
		
		ret = RET_OK;
	}

	return ret;
}

Ret darray_prepend(DArray* thiz, void* data)
{
	return darray_insert(thiz, 0, data);
}

Ret darray_append(DArray* thiz, void* data)
{
	return darray_insert(thiz, -1, data);
}

Ret darray_delete(DArray* thiz, size_t index)
{
	size_t i = 0;

	return_val_if_fail(thiz != NULL && thiz->size > index, RET_INVALID_PARAMS); 

	darray_destroy_data(thiz, thiz->data[index]);
	for(i = index; (i+1) < thiz->size; i++)
	{
		thiz->data[i] = thiz->data[i+1];
	}
	thiz->size--;

	darray_shrink(thiz);

	return RET_OK;
}

Ret darray_get_by_index(DArray* thiz, size_t index, void** data)
{

	return_val_if_fail(thiz != NULL && data != NULL && index < thiz->size, 
		RET_INVALID_PARAMS); 

	*data = thiz->data[index];

	return RET_OK;
}

Ret darray_set_by_index(DArray* thiz, size_t index, void* data)
{
	return_val_if_fail(thiz != NULL && index < thiz->size, 
		RET_INVALID_PARAMS); 

	thiz->data[index] = data;

	return RET_OK;
}

size_t darray_length(DArray* thiz)
{
	return_val_if_fail(thiz != NULL, 0);

	return thiz->size;
}

Ret darray_foreach(DArray* thiz, DataVisitFunc visit, void* ctx)
{
	size_t i = 0;	
	Ret ret = RET_OK;
	return_val_if_fail(thiz != NULL && visit != NULL, RET_INVALID_PARAMS);

	for(i = 0; i < thiz->size; i++)
	{
		ret = visit(ctx, thiz->data[i]);
	}

	return ret;
}

int darray_find(DArray* thiz, DataCompareFunc cmp, void* ctx)
{
	size_t i = 0;

	return_val_if_fail(thiz != NULL && cmp != NULL, -1);

	for (i = 0; i < thiz->size; i++)
	{
		if (cmp(ctx, thiz->data[i]) == 0)
		{
			break;
		}
	}

	return i;
}

void darray_destroy(DArray* thiz)
{
	size_t i = 0;
	
	if (thiz != NULL)
	{
		for (i = 0; i < thiz->size; i++)
		{
			darray_destroy_data(thiz, thiz->data[i]);
		}
		
		SAFE_FREE(thiz->data);
		SAFE_FREE(thiz);
	}

	return;
}

Ret darray_sort(DArray* thiz, SortFunc sort, DataCompareFunc cmp)
{
	return_val_if_fail(thiz != NULL && sort != NULL && cmp != NULL, RET_INVALID_PARAMS);

	return sort(thiz->data, thiz->size, cmp);
}

