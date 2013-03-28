/*
 * File:	handle_word.c
 * Author:	Joe Shang (ID:1101220731)
 * Brief:	Word division module
 */

#include <ctype.h>
#include <stdlib.h>
#include "handle_word.h"

typedef enum
{
	IN_WORD = 0,
	OUT_WORD
}WordState;

typedef struct
{
	int word_tail;
	char word_buf[MAX_WORD_LEN];
}Word;

static void construct_word(Word *word, int character)
{
	word->word_buf[word->word_tail++] = character;
	if (word->word_tail == MAX_WORD_LEN - 1)
	{
		fprintf(stderr, "Detect too long english word, failed to handle it.\n");
		exit(1);
	}
}

void divide_word(FILE *file, OneWordFinishHandler handler, void *ctx)
{
	int c;
	Word word;
	WordState state = OUT_WORD;

	while ((c = fgetc(file)) != EOF)
	{
		/* FSM division process */

		if (state == IN_WORD)
		{
			if (isalnum(c) || c == '_' || c == '-')
			{
				/* Put this character into to word buffer to construct word. */
				construct_word(&word, c);
			}
			else
			{
				state = OUT_WORD;

				/* End the string by '0' */
				construct_word(&word, 0);

				/* Call back the handler when one word finished */
				handler(ctx, word.word_buf);
			}
		}
		else if (state == OUT_WORD)
		{
			if (isalpha(c))
			{
				state = IN_WORD;

				/* New word begin. */
				word.word_tail = 0;
				construct_word(&word, c);
			}
		}
	}
}
