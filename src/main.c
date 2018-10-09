#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <err.h>
#include <stdlib.h>
#include "my_string.h"
#include "commands.h"

struct test tests[NB_FUNC] =
{
  { "-name", &t_name }
};

int handle_elem(struct state *state, struct my_dirent *my_dirent)
{
  for (int i = 0; i < state->funcs_len; i++)
  {
    if (!state->funcs[i].func(my_dirent, state->funcs[i].arg))
      return 0;
  }
  printf("%s\n", my_dirent->path);
  return 1;
}

int listdir(char *path, struct state *state)
{
  struct dirent *dp;
  DIR *dir = opendir(path);
  if (dir == NULL)
  {
    warn("cannot do opendir");
    return 1;
  }
  while ((dp=readdir(dir)) != NULL)
  {
    //printf("debug: %s\n", dp->d_name);
    if (my_strcmp(dp->d_name, ".") && my_strcmp(dp->d_name, ".."))
    {
      char *file_name = dp->d_name;

      char new_path[MAX_PATH];
      my_strcpy(path, new_path);
      my_strcat(new_path, "/");
      my_strcat(new_path, file_name);

      struct stat buf;
      int x;
      if (state->symlink_flag == 1)
        x = stat (new_path, &buf);
      else
        x = lstat (new_path, &buf);
      if (x == -1)
      {
        warn("cannot do stat");
        continue;
      }

      struct my_dirent my_dirent = { dp->d_name, new_path, &buf };

      if (!state->flag_d)
        //printf("%s\n",new_path);
        handle_elem(state, &my_dirent);

      if (S_ISDIR(buf.st_mode))
        listdir(new_path, state);

      if (state->flag_d)
        //printf("%s\n",new_path);
        handle_elem(state, &my_dirent);
    }
  }
  closedir(dir);
  return 0;
}

int main(int argc, char **argv)
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

  struct func *funcs = calloc(argc - i, sizeof(struct func));
  int funcs_len = 0;
  for (; i < argc; i++)
  {
    int k = 0;
    for(; k < NB_FUNC; k++)
    {
      if (my_strcmp(tests[k].name, argv[i]) == 0)
      {
        char *arg = NULL;
        if (argv[i+1] && argv[i+1][0] != '-')
        {
          arg = argv[i+1];
          i++;
        }
        struct func func = { tests[k].func,  arg };
        fprintf(stderr, "d: Add %s with %s\n", tests[k].name, arg);
        funcs[funcs_len] = func;
        funcs_len++;
        break;
      }
    }
    if (k >= NB_FUNC)
      errx(1, "cannot do parsing expr: unknown %s",argv[i]);
  }

  struct state state = { funcs, funcs_len, flag_d, symlink_flag };

  // Launch files
  if (start == end)
  {
    listdir(".", &state);
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
      warn("cannot do stat");
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
      listdir(argv[j], &state);
      if (flag_d)
        printf("%s\n", argv[j]);
    }
  }
}
