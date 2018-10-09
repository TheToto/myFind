#include <fnmatch.h>
#include <err.h>
#include <stdio.h>
#include "my_string.h"
#include "commands.h"

int t_name(struct my_dirent *my_dirent, char *test)
{
  if (test == NULL)
    err(1, "name");
  return fnmatch(test, my_dirent->filename, 0) == 0;
}

int a_print(struct my_dirent *my_dirent, char *test)
{
  if (test != NULL)
    err(1, "print");
  printf("%s\n", my_dirent->path);
}
