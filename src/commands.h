#ifndef COMMANDS_H
#define COMMANDS_H

#define MAX_PATH 4096

struct my_dirent
{
    char *filename;
    char *dirpath;
    char *path;
    struct stat *buf;
};

struct func
{
    int (*func) (struct my_dirent*, struct func*);
    int start;
    int end;
    char **argv;
    struct state *state;
};

struct test
{
    char *name;
    int (*func) (struct my_dirent*, struct func*);
    int hasArg;
    int isAction;
};

struct state
{
    struct expr *expr;
    int flag_d;
    int symlink_flag;
    int argc;
    int hasAction;
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
int a_execdir(struct my_dirent *my_dirent, struct func *func);
int a_delete(struct my_dirent *my_dirent, struct func *func);
int t_perm(struct my_dirent *my_dirent, struct func *func);
int t_user(struct my_dirent *my_dirent, struct func *func);
int t_group(struct my_dirent *my_dirent, struct func *func);
int t_newer(struct my_dirent *my_dirent, struct func *func);

#endif /* !COMMANDS_H */