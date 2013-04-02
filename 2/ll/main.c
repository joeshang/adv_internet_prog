#include <stdio.h>
#include <sys/stat.h>
#include "ll.h"

int main(int argc, char *argv[])
{
	int i;
	struct stat target_stat;

	if (argc == 1) /* No argument, print current directory status */
	{
		print_dir_stat(".");
	}
	else
	{
		for (i = 1; i < argc; i++)
		{
			if (lstat(argv[i], &target_stat) < 0)
			{
				fprintf(stderr, "cannot access %s: no such file or directory\n", argv[i]);
			}
			else
			{
				if (S_ISDIR(target_stat.st_mode))	/* Directory */
				{
					printf("%s:\n", argv[i]);
					print_dir_stat(argv[i]);
				}
				else								/* File */
				{
					print_file_stat(argv[i]);
				}
			}

			if (i != argc - 1)
			{
				printf("\n");
			}
		}
	}

	return 0;
}
