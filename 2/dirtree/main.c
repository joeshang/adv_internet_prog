/*
 * File: main.c
 * Author: Joe Shang (ID:1101220731)
 * Brief: Main program
 */

#include <stdio.h>
#include <stdlib.h>
#include "dirtree.h"

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		print_dir_tree(".", 0);
	}
	else if (argc == 2)
	{
		print_dir_tree(argv[1], 0);
	}
	else
	{
		fprintf(stderr, "Usage: %s [directory]\n", argv[0]);
		exit(1);
	}

	return 0;
}
