#define  _DEFAULT_SOURCE

#include <fnmatch.h>
#include <err.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "my_string.h"
#include "commands.h"

int t_name(struct my_dirent *my_dirent, struct func *func)
{
    if (func->start == func->end)
        errx(1, "cannot do parsing -name: no arg");
    char *arg = func->argv[func->start];
    return fnmatch(arg, my_dirent->filename, 0) == 0;
}

int t_type(struct my_dirent *my_dirent, struct func *func)
{
    if (func->start == func->end)
        errx(1, "cannot do parsing -type: no arg");
    char *arg = func->argv[func->start];
    switch (arg[0])
    {
    case 'b':
        return S_ISBLK(my_dirent->buf->st_mode);
    case 'c':
        return S_ISCHR(my_dirent->buf->st_mode);
    case 'd':
        return S_ISLNK(my_dirent->buf->st_mode);
    case 'p':
        return S_ISFIFO(my_dirent->buf->st_mode);
    case 's':
        return S_ISSOCK(my_dirent->buf->st_mode);
    default:
        break;
    }
    errx(1, "cannot do parsing -type: unknow %s", arg);
}

int a_print(struct my_dirent *my_dirent, struct func *func)
{
    if (func->start != func->end)
        errx (1, "cannot do parsing -print: no arg needed");
    printf("%s\n", my_dirent->path);
    return 1;
}

int a_exec(struct my_dirent *my_dirent, struct func *func)
{
    printf("j'exec %s %s\n", func->argv[func->start], func->argv[func->end-1]);
    return 0;
}
