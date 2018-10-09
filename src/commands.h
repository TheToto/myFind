#define MAX_PATH 4096
#define NB_FUNC 1

struct my_dirent
{
  char *filename;
  char *path;
  struct stat *buf;
};

struct test
{
  char* name;
  int (*func) (struct my_dirent*, char*);
};

struct func
{
  int (*func) (struct my_dirent*, char*);
  char *arg;
};

struct state
{
  struct func *funcs;
  int funcs_len;
  int flag_d;
  int symlink_flag;
};

int t_name(struct my_dirent *my_dirent, char *test);
