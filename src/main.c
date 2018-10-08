#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <err.h>
#include "my_string.h"

#define MAX_PATH 4096

int listdir(char *path, int flag_d, int symlink_flag)
{
  struct dirent *dp;
  DIR *dir = opendir(path);
  if (dir == NULL)
  {
    warn("cannot do opendir");
    return 1;
  }
  while ((dp=readdir(dir)) != NULL) {
    //printf("debug: %s\n", dp->d_name);
    if (my_strcmp(dp->d_name, ".") && my_strcmp(dp->d_name, ".."))
    {
      char *file_name = dp->d_name;

      char new_path[MAX_PATH];
      my_strcpy(path, new_path);
      my_strcat(new_path, "/");
      my_strcat(new_path, file_name);

      if (!flag_d)
        printf("%s\n",new_path);
      struct stat buf;
      int x;
      if (symlink_flag == 1)
        x = stat (new_path, &buf);
      else
        x = lstat (new_path, &buf);
      if (x == -1)
      {
        warn("cannot do lstat");
        continue;
      }
      if (S_ISDIR(buf.st_mode))
        listdir(new_path, flag_d, symlink_flag);
      if (flag_d)
        printf("%s\n",new_path);
    }
  }
  closedir(dir);
  return 0;
}

int main(int argc, char *argv[])
{
  int i = 1;
  int flag_d = 0;
  int symlink_flag = 0;
  for(; i < argc; i++)
  {
    if (argv[i][0] != '-')
      break;
    switch (argv[i][1])
    {
    case 'd':
      flag_d = 1;
      break;
    case 'H':
      symlink_flag = 2;
      break;
    case 'L':
      symlink_flag = 1;
      break;
    case 'P':
      symlink_flag = 0;
      break;
    default:
      errx(1, "cannot do parsing options: unknown %s",argv[i]);
    }
  }
  int start = i;
  for (; i < argc; i++)
  {
    if (argv[i][0] == '-')
      break;
  }
  int end = i;
  for (; i < argc; i++)
  {
    //parsing expr
  }

  if (start == end)
  {
    listdir(".", flag_d, symlink_flag);
  }
  for(int j = start; j < end; j++)
  {
    struct stat buf;
    int x;
    if (symlink_flag > 0)
      x = stat (argv[j], &buf);
    else
      x = lstat (argv[j], &buf);
    if (x == -1)
    {
      warn("cannot do lstat");
      continue;
    }
    if (!S_ISDIR(buf.st_mode))
    {
      printf("%s\n", argv[i]);
    }
    else
    {
      if (!flag_d)
        printf("%s\n", argv[j]);
      listdir(argv[j], flag_d, symlink_flag);
      if (flag_d)
        printf("%s\n", argv[j]);
    }
  }
}
