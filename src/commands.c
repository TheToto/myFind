#include <fnmatch.h>
#include <err.h>
#include <stdio.h>
#include "my_string.h"
#include "commands.h"

int t_name(struct my_dirent *my_dirent, char *arg)
{
    if (arg == NULL)
        errx(1, "cannot do parsing -name: no arg");
    return fnmatch(arg, my_dirent->filename, 0) == 0;
}
/*
int t_type(struct my_dirent *my_dirent, char *arg)
{
    if (arg == NULL)
        errx(1, "cannot do parsing -type: no arg");
    switch (arg[0])
    {
    case 'b':
        return S_ISBLK(my_dirent->stat->st_mode);
    case 'c':
        return S_ISCHR(my_dirent->stat->st_mode);
    case 'd':
        return S_ISLNK(my_dirent->stat->st_mode);
    case 'p':
        return S_ISFIFO(my_dirent->stat->st_mode);
    case 's':
        return S_ISSOCK(my_dirent->stat->st_mode);
    default:
        break;
    }
    errx(1, "cannot do parsing -type: unknow %s", arg);
}
*/
int a_print(struct my_dirent *my_dirent, char *arg)
{
    if (arg != NULL)
        errx (1, "cannot do parsing -print: no arg needed");
    printf("%s\n", my_dirent->path);
    return 1;
}
