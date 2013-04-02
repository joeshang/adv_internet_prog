/*
 * File: ll.c
 * Author: Joe Shang (ID:1101220731)
 * Brief: Implementation of printing file/directory detail informantion.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

#define MODE_STR_LEN	MODE_END_POS + 1

enum
{
	MODE_TYPE_POS = 0,
	MODE_RUSR_POS,
	MODE_WUSR_POS,
	MODE_XUSR_POS,
	MODE_RGRP_POS,
	MODE_WGRP_POS,
	MODE_XGRP_POS,
	MODE_ROTH_POS,
	MODE_WOTH_POS,
	MODE_XOTH_POS,
	MODE_END_POS,
};

static int get_mode_string(mode_t mode, char *mode_string)
{
	int i;
	int ret = 0;

	/* Initialize the mode string buffer */
	for (i = MODE_TYPE_POS; i < MODE_END_POS; i++)
	{
		mode_string[i] = '-';
	}
	mode_string[MODE_END_POS] = '\0';

	/* File type */
	if (S_ISREG(mode))			/* Regular file */
	{
		mode_string[MODE_TYPE_POS] = '-';
	}
	else if (S_ISDIR(mode))		/* Directory */
	{
		mode_string[MODE_TYPE_POS] = 'd';
	}
	else if (S_ISCHR(mode))		/* Character special file */
	{
		mode_string[MODE_TYPE_POS] = 'c';
	}
	else if (S_ISBLK(mode))		/* Block special file */
	{
		mode_string[MODE_TYPE_POS] = 'b';
	}
	else if (S_ISFIFO(mode))	/* FIFO file */
	{
		mode_string[MODE_TYPE_POS] = 'p';
	}
	else if (S_ISLNK(mode))		/* Symbol link file */
	{
		mode_string[MODE_TYPE_POS] = 'l';
	}
	else if (S_ISSOCK(mode))	/* Socket file */
	{
		mode_string[MODE_TYPE_POS] = 's';
	}
	else						/* Unknown type file */
	{
		fprintf(stderr, "Unknown file type.\n");
		return -1;
	}

	/* File access permission */
	/* User */
	if (S_IRUSR & mode)
	{
		mode_string[MODE_RUSR_POS] = 'r';
	}
	if (S_IWUSR & mode)
	{
		mode_string[MODE_WUSR_POS] = 'w';
	}
	if (S_IXUSR & mode)
	{
		mode_string[MODE_XUSR_POS] = 'x';
	}

	/* Group */
	if (S_IRGRP & mode)
	{
		mode_string[MODE_RGRP_POS] = 'r';
	}
	if (S_IWGRP & mode)
	{
		mode_string[MODE_WGRP_POS] = 'w';
	}
	if (S_IXGRP & mode)
	{
		mode_string[MODE_XGRP_POS] = 'x';
	}

	/* Other */
	if (S_IROTH & mode)
	{
		mode_string[MODE_ROTH_POS] = 'r';
	}
	if (S_IWOTH & mode)
	{
		mode_string[MODE_WOTH_POS] = 'w';
	}
	if (S_IXOTH & mode)
	{
		mode_string[MODE_XOTH_POS] = 'x';
	}

	return ret;
}

void print_file_stat(char *file_name)
{
	struct stat file_stat;
	char mode_string[MODE_STR_LEN];
	struct tm *time;
	char time_buf[256];
	struct passwd *pwd;
	struct group *grp;

	/* Get file information structure */
	if (lstat(file_name, &file_stat) < 0)
	{
		fprintf(stderr, "cannot access %s: no such file or directory\n", file_name);
		return;
	}
		
	/* Mode */
	if (get_mode_string(file_stat.st_mode, mode_string) < 0)
	{
		exit(1);
	}
	printf("%s ", mode_string);

	/* Link count */
	printf("%ld ", (long)file_stat.st_nlink);

	/* Owner */
	pwd = getpwuid(file_stat.st_uid);
	printf("%s ", pwd->pw_name);

	/* Group */
	grp = getgrgid(file_stat.st_gid);
	printf("%s ", grp->gr_name);

	/* File Size */
	printf("%4lld ", (long long)file_stat.st_size);

	/* Time of last modificaiton */
	time = localtime(&(file_stat.st_mtime));
	strftime(time_buf, 256, "%b %d %H:%M", time);
	printf("%s ", time_buf);

	printf("%s", file_name);

	printf("\n");
}

void print_dir_stat(char *dir_name)
{
	DIR *dp;
	struct dirent *entry;
	char *file_path;
	
	if ((dp = opendir(dir_name)) == NULL)
	{
		fprintf(stderr, "cannot open directory: %s\n", dir_name);
		return;
	}

	while ((entry = readdir(dp)) != NULL)
	{
		file_path = (char *)malloc((strlen(dir_name) + 1 + strlen(entry->d_name) + 1) * sizeof(char));
		strcpy(file_path, dir_name);
		strcat(file_path, "/");
		strcat(file_path, entry->d_name);

		print_file_stat(file_path);

		free(file_path);
		file_path = NULL;
	}

	closedir(dp);
}

