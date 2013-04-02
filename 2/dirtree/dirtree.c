/*
 * File: dirtree.c
 * Author: Joe Shang (ID:1101220731)
 * Brief: Printing "tree" architecture of directory
 */

#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "dirtree.h"

void print_dir_tree(char *dir_name, int depth)
{
	DIR *dp;
	struct dirent *entry;
	struct stat file_stat;

	if ((dp = opendir(dir_name)) == NULL)
	{
		fprintf(stderr, "cannot open directory: %s\n", dir_name);
		return;
	}

	/* Switch target directroy */
	chdir(dir_name);

	while ((entry = readdir(dp)) != NULL)
	{
		lstat(entry->d_name, &file_stat);
		if (S_ISDIR(file_stat.st_mode))
		{
			/* Ignore . and .. */
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			{
				continue;
			}

			printf("%*s%s%s\n", depth, "", "|--> ", entry->d_name);

			/* Directory, recurse it with new indent */
			print_dir_tree(entry->d_name, depth + 4);
		}
		else
		{
			printf("%*s%s%s\n", depth, "", "|--> ", entry->d_name);
		}
	}

	/* Back to upper level directory */
	chdir("..");

	closedir(dp);
}
