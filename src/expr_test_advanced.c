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
#include "parsing.h"

struct my_perm
{
    unsigned int wusr;
    unsigned int xusr;
    unsigned int rusr;
    unsigned int rgrp;
    unsigned int wgrp;
    unsigned int xgrp;
    unsigned int roth;
    unsigned int woth;
    unsigned int xoth;
};

static struct my_perm create_perm_file(struct stat *buf)
{
    struct my_perm perm;
    perm.rusr = (buf->st_mode & S_IRUSR) ? 1 : 0;
    perm.wusr = (buf->st_mode & S_IWUSR) ? 1 : 0;
    perm.xusr = (buf->st_mode & S_IXUSR) ? 1 : 0;
    perm.rgrp = (buf->st_mode & S_IRGRP) ? 1 : 0;
    perm.wgrp = (buf->st_mode & S_IWGRP) ? 1 : 0;
    perm.xgrp = (buf->st_mode & S_IXGRP) ? 1 : 0;
    perm.roth = (buf->st_mode & S_IROTH) ? 1 : 0;
    perm.woth = (buf->st_mode & S_IWOTH) ? 1 : 0;
    perm.xoth = (buf->st_mode & S_IXOTH) ? 1 : 0;
    return perm;
}

static struct my_perm create_perm_string(char *arg)
{
    struct my_perm perm;
    perm.rusr = ((arg[0] -'0') & 4) ? 1 : 0;
    perm.wusr = ((arg[0] -'0') & 2) ? 1 : 0;
    perm.xusr = ((arg[0] -'0') & 1) ? 1 : 0;
    perm.rgrp = ((arg[1] -'0') & 4) ? 1 : 0;
    perm.wgrp = ((arg[1] -'0') & 2) ? 1 : 0;
    perm.xgrp = ((arg[1] -'0') & 1) ? 1 : 0;
    perm.roth = ((arg[2] -'0') & 4) ? 1 : 0;
    perm.woth = ((arg[2] -'0') & 2) ? 1 : 0;
    perm.xoth = ((arg[2] -'0') & 1) ? 1 : 0;
    return perm;
}

int t_perm(struct my_dirent *my_dirent, struct func *func)
{
    char *arg = func->argv[func->start];
    if (arg[0] < '0' || arg[0] > '9')
        arg++;

    struct my_perm f = create_perm_file(my_dirent->buf);
    struct my_perm s = create_perm_string(arg);

    if (fnmatch("???", func->argv[func->start], 0) == 0)
    {
        if ((f.rusr == s.rusr) && (f.wusr == s.wusr) && (f.xusr == s.xusr)
            && (f.rgrp == s.rgrp) && (f.wgrp == s.wgrp) && (f.xgrp == s.xgrp)
            && (f.roth == s.roth) && (f.woth == s.woth) && (f.xoth == s.xoth))
            return 1;
    }
    else if (fnmatch("-???", func->argv[func->start], 0) == 0)
    {
        if ((f.rusr || !s.rusr) && (f.wusr || !s.wusr) && (f.xusr || !s.xusr)
            && (f.rgrp || !s.rgrp) && (f.wgrp || !s.wgrp) && (f.xgrp || !s.xgrp)
            && (f.roth || !s.roth) && (f.woth || !s.woth) && (f.xoth || !s.xoth))
            return 1;
    }
    else if (fnmatch("/???", func->argv[func->start], 0) == 0)
    {
        if ((f.rusr && s.rusr) || (f.wusr && s.wusr) || (f.xusr && s.xusr)
            || (f.rgrp && s.rgrp) || (f.wgrp && s.wgrp) || (f.xgrp && s.xgrp)
            || (f.roth && s.roth) || (f.woth && s.woth) || (f.xoth && s.xoth))
            return 1;
    }
    else
    {
        errx(1, "cannot do parsing perm: incorrect string");
    }
    return 0;
}

int t_newer(struct my_dirent *my_dirent, struct func *func)
{
    char *arg = func->argv[func->start];
    struct stat buf;
    int res = make_stat(&buf, func->state, arg);
    if (res == -1)
        err(1, "cannot do -newer");
    time_t newer = buf.st_mtime;
    if (newer < my_dirent->buf->st_mtime)
        return 1;
    return 0;
}