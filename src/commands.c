#include <fnmatch.h>
#include "my_string.h"
#include "commands.h"

int t_name(struct my_dirent *my_dirent, char *test)
{
  return fnmatch(test, my_dirent->filename, 0) == 0;
}

//int a_print(char *filename)
