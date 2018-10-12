#define  _DEFAULT_SOURCE

#include <fnmatch.h>
#include <err.h>
#include <stdio.h>
#include <sys/types.h>
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
    char *arg = func->argv[func->start];
    switch (arg[0])
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
    pid_t pid = fork();

    if (pid == -1)
        err(1, "cannot do exec");
    if (pid == 0)
    {
        char *command = func->argv[func->start];
        func->argv[func->end-1] = NULL;
        char **args = func->argv + func->start;
        for(int i = 0; args[i] != NULL; i++)
        {
            if (my_strcmp(args[i], "{}") == 0)
                args[i] = my_dirent->path;
        }
        execvp(command, args);
        err(1, "cannot do exec");
    }
    else
    {
        int status = 0;
        waitpid(pid, &status, 0);
        return status == 0;
    }
    return 0;
}

int a_execdir(struct my_dirent *my_dirent, struct func *func)
{
    char cur_path[MAX_PATH] = { 0 };
    getcwd(cur_path, MAX_PATH);
    if (cur_path == NULL)
        err(1, "cannot do execdir");
    int status = chdir(my_dirent->dirpath);
    if (status == -1)
        err(1, "cannot do execdir");
    char new_fullpath[MAX_PATH] = { 0 };
    my_strcat(new_fullpath, "./");
    my_strcat(new_fullpath, my_dirent->filename);
    my_dirent->path = new_fullpath;
    int res = a_exec(my_dirent, func);
    status = chdir(cur_path);
    if (status == -1)
        err(1, "cannot do execdir");
    return res;
}

static int remove_directory(const char *path)
{
    fprintf(stderr, "m:%s\n", path);
    DIR *d = opendir(path);
    if (!d)
        err(1,"%s", path);
    size_t path_len = my_strlen(path);
    int r = -1;
    if (d != NULL)
    {
        struct dirent *p;
        r = 0;
        while (r != -1 && (p=readdir(d)))
        {
            int r2 = -1;
            if (!my_strcmp(p->d_name, ".") || !my_strcmp(p->d_name, ".."))
                continue;
            size_t len = path_len + my_strlen(p->d_name) + 2; 
            char *buf = calloc(len, sizeof(char));
            if (buf != NULL)
            {
                struct stat statbuf;
                my_strcat(buf, path);
                my_strcat(buf, "/");
                my_strcat(buf, p->d_name);
                if (!lstat(buf, &statbuf))
                {
                    if (S_ISDIR(statbuf.st_mode))
                        r2 = remove_directory(buf);
                    else
                        r2 = unlink(buf);
                }
                if (r2 == -1)
                    err(1,"o:%s\n", buf);
                free(buf);
            }
            r = r2;
        }
        closedir(d);
    }
    if (r != -1)
        r = rmdir(path);
    return r;
}

int a_delete(struct my_dirent *my_dirent, struct func *func)
{
    if (func->start != func->end)
        errx (1, "cannot do parsing -print: no arg needed");
    int res;
    struct stat buf;
    lstat(my_dirent->path, &buf);
    if (S_ISDIR(buf.st_mode))
        res = remove_directory(my_dirent->path);
    else
        res = unlink(my_dirent->path);
    if (res == -1)
        err(1, "cannot do delete");
    return 1;
}

int t_perm(struct my_dirent *my_dirent, struct func *func)
{
    return 1;
}

int t_user(struct my_dirent *my_dirent, struct func *func)
{
    return 1;
}

int t_group(struct my_dirent *my_dirent, struct func *func)
{
    return 1;
}
int t_newer(struct my_dirent *my_dirent, struct func *func)
{
    return 1;
}
