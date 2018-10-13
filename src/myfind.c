#define _DEFAULT_SOURCE

#include <stdio.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <err.h>
#include <stdlib.h>
#include "my_string.h"
#include "commands.h"
#include "parsing.h"

static int evaluate_expr(struct expr *expr, struct state *state,
        struct my_dirent *my_dirent)
{
    int argc = state->argc;
    if (!expr->expr)
        return expr->func->func(my_dirent, expr->func);
    int not_next = 0;
    if (expr->expr[0] == NULL)
        return 1;
    for (int line = 0; line < argc; line++)
    {
        if (expr->expr[line*argc+0] == NULL)
            break;
        for (int col = 0; col < argc; col++)
        {
            if (expr->expr[line*argc+col] == NULL)
                return 1;
            if (expr->expr[line*argc+col]->func &&
                    expr->expr[line*argc+col]->func->start == -1)
            {
                not_next = 1;
                continue;
            }
            int r = evaluate_expr(expr->expr[line*argc+col], state, my_dirent);
            if ((r && !not_next) || (!r && not_next))
            {
                not_next = 0;
                continue;
            }
            else
            {
                break;
            }
        }
    }
    return 0;
}

static int handle_elem(struct state *state, struct my_dirent *my_dirent)
{
    int res = evaluate_expr(state->expr, state, my_dirent);
    if (state->hasAction == 0 && res)
        printf("%s\n", my_dirent->path);
    return res;
}

static void listdir(char *path, struct state *state)
{
    struct dirent *dp;
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        warn("cannot do opendir");
        return;
    }
    while ((dp=readdir(dir)) != NULL)
    {
        if (my_strcmp(dp->d_name, ".") && my_strcmp(dp->d_name, ".."))
        {
            char new_path[MAX_PATH];
            my_strcpy(path, new_path);
            int l = my_strlen(new_path);
            if (new_path[l-1] != '/')
                my_strcat(new_path, "/");
            my_strcat(new_path, dp->d_name);

            struct stat buf;
            int x = make_stat(&buf, state, new_path);
            if (x == -1)
            {
                warn("cannot do stat");
                continue;
            }

            struct my_dirent my_dirent = { dp->d_name, path, new_path, &buf };

            if (!state->flag_d)
                handle_elem(state, &my_dirent);

            x = make_stat(&buf, state, new_path);
            if (x != -1 && S_ISDIR(buf.st_mode))
                listdir(new_path, state);

            if (state->flag_d)
                handle_elem(state, &my_dirent);
        }
    }
    closedir(dir);
}

static void launch_args(struct state *state, int start, int end, char **argv)
{
    for(int j = start; j < end; j++)
    {
        struct stat buf;
        int x = make_stat(&buf, state, argv[j]);
        if (x == -1)
        {
            warn("cannot do stat");
            continue;
        }
        char res[MAX_PATH] = { 0 };
        char res2[MAX_PATH] = { 0 };
        struct my_dirent my_dirent = { my_filename(argv[j], res2),
            my_dirname(argv[j], res), argv[j], &buf };
        if (!S_ISDIR(buf.st_mode))
        {
            handle_elem(state, &my_dirent);
        }
        else
        {
            if (!state->flag_d)
                handle_elem(state, &my_dirent);
            x = make_stat(&buf, state, argv[j]);
            if (x != -1)
                listdir(argv[j], state);
            if (state->flag_d)
                handle_elem(state, &my_dirent);
        }
    }
}

static void launch_noargs(struct state *state)
{
        struct stat buf;
        int x;
        if (state->symlink_flag > 0)
            x = stat (".", &buf);
        else
            x = lstat (".", &buf);
        if (x == -1)
        {
            warn("cannot do stat");
        }
        else
        {
            struct my_dirent my_dirent = { ".", ".", ".", &buf };
            if (!state->flag_d)
                handle_elem(state, &my_dirent);
            listdir(".", state);
            if (state->flag_d)
                handle_elem(state, &my_dirent);
        }
}

int main(int argc, char **argv)
{
    int i = 1;
    struct state state = { NULL, 0, 0, argc, 0 };
    parse_flag(&i, &state, argc, argv);
    int start = i;
    for (; i < argc; i++)
    {
        if (argv[i][0] == '-' || argv[i][0] == '(' || argv[i][0] == ')')
            break;
    }
    int end = i;
    state.expr = parse_expr(&i , argv, argc, &state);
    // Launch files
    if (start == end)
        launch_noargs(&state);
    else
        launch_args(&state, start, end, argv);
    free_expr(state.expr, &state);
    return 0;
}