#define  _DEFAULT_SOURCE

#include <fnmatch.h>
#include <err.h>
#include <stdio.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
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
    switch (func->argv[func->start][0])
    {
    case 'f':
        return S_ISREG(my_dirent->buf->st_mode);
    case 'b':
        return S_ISBLK(my_dirent->buf->st_mode);
    case 'c':
        return S_ISCHR(my_dirent->buf->st_mode);
    case 'd':
        return S_ISDIR(my_dirent->buf->st_mode);
    case 'p':
        return S_ISFIFO(my_dirent->buf->st_mode);
    case 's':
        return S_ISSOCK(my_dirent->buf->st_mode);
    default:
        break;
    }
    errx(1, "cannot do parsing -type: unknow %s", func->argv[func->start]);
}

int t_user(struct my_dirent *my_dirent, struct func *func)
{
    char *arg = func->argv[func->start];
    struct passwd *pw = getpwnam(arg);
    if (!pw)
        return 0;
    if (pw->pw_uid == my_dirent->buf->st_uid)
        return 1;
    return 0;
}

int t_group(struct my_dirent *my_dirent, struct func *func)
{
    char *arg = func->argv[func->start];
    struct group *gr = getgrnam(arg);
    if (!gr)
        return 0;
    if (gr->gr_gid == my_dirent->buf->st_gid)
        return 1;
    return 0;
}