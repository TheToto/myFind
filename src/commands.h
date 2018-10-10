#define MAX_PATH 4096

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
  struct expr *expr;
  int flag_d;
  int symlink_flag;
};

struct expr
{
    struct func *func;
    struct expr **expr;
};

int t_name(struct my_dirent *my_dirent, char *test);
int a_print(struct my_dirent *my_dirent, char *test);
