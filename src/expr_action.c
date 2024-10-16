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

static int remove_directory(const char *path, struct state *state)
{
    DIR *d = opendir(path);
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
            char buf[MAX_PATH] = { 0 };
            struct stat statbuf;
            my_strcat(buf, path);
            my_strcat(buf, "/");
            my_strcat(buf, p->d_name);
            if (!make_stat(&statbuf, state, buf))
            {
                if (S_ISDIR(statbuf.st_mode))
                    r2 = remove_directory(buf, state);
                else
                    r2 = unlink(buf);
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
    struct stat buf;
    int res = make_stat(&buf, func->state, my_dirent->path);
    if (res == -1)
      err(1, "cannot do delete");
    if (S_ISDIR(buf.st_mode))
        res = remove_directory(my_dirent->path, func->state);
    else
        res = unlink(my_dirent->path);
    if (res == -1)
        errx(1, "cannot do delete: no perm");
    return 1;
}
