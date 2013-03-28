/*
 * File:	main.c
 * Author:	Joe Shang (ID:1101220731)
 * Brief:	The main process in program
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "darray.h"
#include "handle_word.h"

#define RESULT_FILE	"result.txt"

typedef struct
{
	char *word;
	int freq;
}WordElement;

static void word_destroy(void *ctx, void *data)
{
	WordElement *element = (WordElement *)data;
	SAFE_FREE(element->word);
	SAFE_FREE(element);
}

static int cmp_word_freq(void *p1, void *p2)
{
	return ((*(WordElement **)p2)->freq - (*(WordElement **)p1)->freq);
}

static int cmp_word_string(void *ctx, void *data)
{
	return strcmp((char *)ctx, ((WordElement *)data)->word);
}

static Ret qsort_wrapper(void **array, size_t nr, DataCompareFunc cmp)
{
	qsort(array, nr, sizeof(void *), cmp);

	return RET_OK;
}

static Ret print_element(void *ctx, void *data)
{
	WordElement *word_element = data;
	printf("%15s:%4d\n", word_element->word, word_element->freq);
	fprintf((FILE *)ctx, "%15s:%4d\n", word_element->word, word_element->freq);
	return RET_OK;
}

/* One word finish, match and update word frequecy database. */
static void update_database(void *ctx, char *string)
{
	int index;
	WordElement *word_element;
	DArray *word_array = (DArray *)ctx;

	/* No match word in database, create a new element */
	if ((index = darray_find(word_array, cmp_word_string, string)) == darray_length(word_array))
	{
		/* Construct word element of database */
		char *buf	= (char *)malloc(strlen(string) + 1);
		strcpy(buf, string);
		if (buf == NULL)
		{
			fprintf(stderr, "Failed to malloc, no enough memory space.\n");
			exit(1);
		}

		word_element = (WordElement *)malloc(sizeof(WordElement));
		if (word_element == NULL)
		{
			fprintf(stderr, "Failed to malloc, no enough memory space.\n");
			free(buf);
			exit(1);
		}
		word_element->word = buf;
		word_element->freq = 1;

		darray_append(word_array, word_element);
	}
	/* Matching word in database, update the frequency of that word */
	else
	{
		darray_get_by_index(word_array, index, (void **)&word_element);
		word_element->freq++;
		darray_set_by_index(word_array, index, (void *)word_element);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		return 1;
	}

	/* Initialize word database */
	DArray *word_array = darray_create(word_destroy, NULL);
	
	/* Open target file */
	FILE *word_file = fopen(argv[1], "r");
	FILE *result_file = fopen(RESULT_FILE, "w+");
	if (word_file == NULL)
	{
		fprintf(stderr, "Failed to open file %s\n", argv[1]);
		return 1;
	}

	/* Divide word and put it into database to update word frequency */
	divide_word(word_file, update_database, word_array);

	/* Close target file */
	fclose(word_file);

	/* Sort words in database by word frequency */
	darray_sort(word_array, qsort_wrapper, cmp_word_freq);

	/* Output final result */
	darray_foreach(word_array, print_element, result_file);

	darray_destroy(word_array);
	fclose(result_file);

	return 0;
}

