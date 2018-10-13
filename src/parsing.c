#define _DEFAULT_SOURCE

#include <stdio.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <err.h>
#include <stdlib.h>
#include "commands.h"
#include "my_string.h"
#include "parsing.h"

struct test tests[NB_FUNC] =
{
    { "-name", &t_name, 1, 0 },
    { "-print", &a_print, 0, 1 },
    { "-type", &t_type, 1, 0 },
    { "-exec", &a_exec, 1, 1 },
    { "-execdir", &a_execdir, 1, 1 },
    { "-delete", &a_delete, 0, 1 },
    { "-perm", &t_perm, 1, 0 },
    { "-user", &t_user, 1, 0 },
    { "-group", &t_group, 1, 0 },
    { "-newer", &t_newer, 1, 0 }
};

int make_stat(struct stat *buf, struct state *state, char *arg)
{
    int x;
    if (state->symlink_flag == 1)
        x = stat (arg, buf);
    else
        x = lstat (arg, buf);
    return x;
}

void free_expr(struct expr *expr, struct state *state)
{
    if (!expr)
        return;
    int argc = state->argc;
    if (!expr->expr)
    {
        free(expr->func);
    }
    else
    {
        for (int i = 0; i < argc * 2; i++)
            free_expr(expr->expr[i], state);
        free(expr->expr);
    }
    free(expr);
}

void parse_flag(int *i, struct state *state, int argc, char **argv)
{
    int quit = 0;
    for(; *i < argc && !quit; i++)
    {
        if (argv[*i][0] != '-')
            break;
        switch (argv[*i][1])
        {
        case 'd':
            state->flag_d = 1;
            break;
        case 'H':
            state->symlink_flag = 2;
            break;
        case 'L':
            state->symlink_flag = 1;
            break;
        case 'P':
            state->symlink_flag = 0;
            break;
        default:
            quit = 1;
            (*i)--;
            break;
        }
    }
}

struct expr *parse_expr(int *i, char **argv, int argc, struct state *state)
{
    struct expr *expr = malloc(sizeof(struct expr));
    if (!expr)
        err(1, "cannot do malloc");
    if (!state->expr)
    {
        state->expr = expr;
    }
    expr->expr = calloc(argc * argc, sizeof(struct expr*));
    if (!expr->expr)
        err(1, "cannot do malloc");
    expr->func = NULL;
    int line = 0;
    int col = 0;
    for (; *i < argc; (*i)++)
    {
        if (my_strcmp(argv[*i], "(") == 0)
        {
            (*i)++;
            expr->expr[line*argc+col] = parse_expr(i, argv, argc, state);
            col++;
            continue;
        }
        if (my_strcmp(argv[*i], ")") == 0)
            break;
        if (my_strcmp(argv[*i], "-a") == 0)
            continue;
        if (my_strcmp(argv[*i], "-o") == 0)
        {
            line++;
            col = 0;
            continue;
        }
        if (my_strcmp(argv[*i], "!") == 0)
        {
            struct func *func = malloc(sizeof(struct func));
            if (!func)
                err(1, "cannot do malloc");
            func->start = -1;
            struct expr *new = malloc(sizeof(struct expr));
            if (!new)
                err(1, "cannot do malloc");
            new->expr = NULL;
            new->func = func;
            expr->expr[line*argc+col] = new;

            col++;
            continue;
        }
        int k = 0;
        for(; k < NB_FUNC; k++)
        {
            if (my_strcmp(tests[k].name, argv[*i]) == 0)
            {
                int start_arg = *i+1;
                if (my_strcmp("-exec", tests[k].name) == 0 ||
                        my_strcmp("-execdir", tests[k].name) == 0)
                {
                    while (*i+1 < argc && my_strcmp(argv[*i+1], ";") == 1 &&
                            my_strcmp(argv[*i], "+") == 1)
                    {
                        (*i)++;
                    }
                    if (*i+1 >= argc)
                    {
                      free_expr(expr, state);
                      free_expr(state->expr, state);
                      errx(1, "cannot do parsing exec arg: no end");
                    }
                    (*i)++;
                }
                else
                {
                    if (tests[k].hasArg)
                    {
                        (*i)++;
                    }
                }
                int end_arg = *i+1;
                if (*i >= argc)
                {
                    free_expr(expr, state);
                    free_expr(state->expr, state);
                    errx(1, "cannot do parsing %s: unknow arg", tests[k].name);
                }
                struct func *func = malloc(sizeof(struct func));
                if (!func)
                    err(1, "cannot do malloc");
                if (tests[k].isAction)
                    state->hasAction = 1;
                func->func = tests[k].func;
                func->start = start_arg;
                func->end = end_arg;
                func->argv = argv;
                func->state = state;
                struct expr *new = malloc(sizeof(struct expr));
                if (!new)
                    err(1, "cannot do malloc");
                new->expr = NULL;
                new->func = func;
                expr->expr[line*argc+col] = new;
                col++;
                break;
            }
        }
        if (k >= NB_FUNC)
        {
            free_expr(expr, state);
            free_expr(state->expr, state);
            errx(1, "cannot do parsing %s: unknown expr",argv[*i]);
        }
    }
    return expr;
}