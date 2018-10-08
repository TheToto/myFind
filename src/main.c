#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "my_string.h"

#define MAX_PATH 256

void listdir(char *path)
{
  struct dirent *dp;
  DIR *dir = opendir(path);
  while ((dp=readdir(dir)) != NULL) {
    //printf("debug: %s\n", dp->d_name);
    if (my_strcmp(dp->d_name, ".") && my_strcmp(dp->d_name, ".."))
    {
      char next_path[MAX_PATH];
      char *file_name = dp->d_name;
      printf("%s/%s\n",path,file_name);
      if (dp->d_type == DT_DIR)
      {
        my_strcpy(path, next_path);
        my_strcat(next_path, "/");
        my_strcat(next_path, file_name);
        listdir(next_path);
      }
    }
  }
  closedir(dir);
  return;
}

int main(int argc, char *argv[])
{
  int i = 1;
  for(; i < argc; i++)
  {
    if (argv[i][0] != '-')
      break;
    switch (argv[i][1])
    {
    case 'd':
      break;
    case 'H':
      break;
    case 'L':
      break;
    case 'P':
      break;
    }
  }
  if (argc == 1)
  {
    listdir(".");
  }
  for(; i < argc; i++)
  {
    struct stat buf;
    int x = lstat (argv[i], &buf);
    if (S_ISLNK(buf.st_mode))
    {
      printf("%s\n", argv[i]);
    }
    else
    {
      listdir(argv[i]);
    }
  }
}
