#define MAX_PATH 4096

struct my_dirent
{
    char *filename;
    char *path;
    struct stat *buf;
};

struct func
{
    int (*func) (struct my_dirent*, struct func*);
    int start;
    int end;
    char **argv;
};

struct test
{
    char *name;
    int (*func) (struct my_dirent*, struct func*);
    int hasArg;
};

struct state
{
    struct expr *expr;
    int flag_d;
    int symlink_flag;
    int argc;
};

struct expr
{
    struct func *func;
    struct expr **expr;
};

int t_name(struct my_dirent *my_dirent, struct func *func);
int a_print(struct my_dirent *my_dirent, struct func *func);
int t_type(struct my_dirent *my_dirent, struct func *func);
int a_exec(struct my_dirent *my_dirent, struct func *func);
