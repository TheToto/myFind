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

#define NB_FUNC 10

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

int evaluate_expr(struct expr *expr, struct state *state,
        struct my_dirent *my_dirent)
{
    int argc = state->argc;
    if (!expr->expr)
        return expr->func->func(my_dirent, expr->func);
    int line = 0;
    int col = 0;
    int not_next = 0;
    if (expr->expr[0] == NULL)
        return 1;
    for (; line < argc; line++)
    {
        if (expr->expr[line*argc+col] == NULL)
            break;
        for (; col < argc; col++)
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
                break;
        }
    }
    return 0;
}

int handle_elem(struct state *state, struct my_dirent *my_dirent)
{
    int res = evaluate_expr(state->expr, state, my_dirent);
    if (!state->hasAction && res)
        printf("%s\n", my_dirent->path);
    return res;
}

int listdir(char *path, struct state *state)
{
    struct dirent *dp;
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        warn("cannot do opendir");
        return 1;
    }
    while ((dp=readdir(dir)) != NULL)
    {
        //printf("debug: %s\n", dp->d_name);
        if (my_strcmp(dp->d_name, ".") && my_strcmp(dp->d_name, ".."))
        {
            char *file_name = dp->d_name;

            char new_path[MAX_PATH];
            my_strcpy(path, new_path);
            int l = my_strlen(new_path);
            if (new_path[l-1] != '/')
                my_strcat(new_path, "/");
            my_strcat(new_path, file_name);

            struct stat buf;
            int x;
            if (state->symlink_flag == 1)
                x = stat (new_path, &buf);
            else
                x = lstat (new_path, &buf);
            if (x == -1)
            {
                warn("cannot do stat");
                continue;
            }

            struct my_dirent my_dirent = { dp->d_name, path, new_path, &buf };

            if (!state->flag_d)
                handle_elem(state, &my_dirent);

            if (state->symlink_flag == 1)
                x = stat (new_path, &buf);
            else
                x = lstat (new_path, &buf);
            if (x != -1 && S_ISDIR(buf.st_mode))
                listdir(new_path, state);

            if (state->flag_d)
                handle_elem(state, &my_dirent);
        }
    }
    closedir(dir);
    return 0;
}

struct expr *parse_expr(int *i, char **argv, int argc, int *hasAction)
{
    fprintf(stderr, "d: %d Enter un (\n", *i);
    struct expr *expr = malloc(sizeof(struct expr));
    // HANDLE MALLOC ERRORS
    expr->expr = calloc(argc * argc, sizeof(struct expr*));
    // AGAIN
    expr->func = NULL;
    int line = 0;
    int col = 0;
    for (; *i < argc; (*i)++)
    {
        fprintf(stderr, "dd: Parse %s at %d\n", argv[*i], *i);
        if (my_strcmp(argv[*i], "(") == 0)
        {
            (*i)++;
            expr->expr[line*argc+col] = parse_expr(i, argv, argc, hasAction);
            col++;
            continue;
        }
        if (my_strcmp(argv[*i], ")") == 0)
        {
            fprintf(stderr, "d: %i Close )\n",*i);
            break;
        }
        if (my_strcmp(argv[*i], "-a") == 0)
            continue;
        if (my_strcmp(argv[*i], "-o") == 0)
        {
            line++;
            col = 0;
            fprintf(stderr, "d: -o : new line %d\n", line);
            continue;
        }
        if (my_strcmp(argv[*i], "!") == 0)
        {
            struct func *func = malloc(sizeof(struct func));
            // AGAIN
            func->start = -1;
            fprintf(stderr, "d: Add ! with -1\n");
            struct expr *new = malloc(sizeof(struct expr));
            // AGAIN
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
                        errx(1, "cannot do parsing exec arg: no end");
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
                    // FREE ALL EXPR ALREADY ALOCATED !
                    errx(1, "cannot do parsing %s: unknow arg", tests[k].name);
                }
                struct func *func = malloc(sizeof(struct func));
                // AGAIN
                if (tests[k].isAction)
                    *hasAction = 1;
                func->func = tests[k].func;
                func->start = start_arg;
                func->end = end_arg;
                func->argv = argv;
                fprintf(stderr, "d: Add %s with %s\n", tests[k].name, argv[start_arg]);
                struct expr *new = malloc(sizeof(struct expr));
                // AGAIN
                new->expr = NULL;
                new->func = func;
                expr->expr[line*argc+col] = new;
                col++;
                break;
            }
        }
        if (k >= NB_FUNC)
            errx(1, "cannot do parsing %s: unknown expr",argv[*i]);
    }
    return expr;
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

int main(int argc, char **argv)
{
    //fprintf(stderr,"d: argc: %d\n", argc);
    int i = 1;
    int flag_d = 0;
    int symlink_flag = 0;
    int quit = 0;
    for(; i < argc && !quit; i++)
    {
        if (argv[i][0] != '-')
            break;
        switch (argv[i][1])
        {
        case 'd':
            flag_d = 1;
            break;
        case 'H':
            symlink_flag = 2;
            break;
        case 'L':
            symlink_flag = 1;
            break;
        case 'P':
            symlink_flag = 0;
            break;
        default:
            quit = 1;
            i--;
            break;
        }
    }
    int start = i;
    for (; i < argc; i++)
    {
        if (argv[i][0] == '-' || argv[i][0] == '(' || argv[i][0] == ')')
            break;
    }
    int end = i;
    int hasAction = 0;
    struct expr *expr = parse_expr(&i , argv, argc, &hasAction);
    struct state state = { expr, flag_d, symlink_flag, argc, hasAction };

    // Launch files
    if (start == end)
    {
        struct stat buf;
        int x;
        if (symlink_flag > 0)
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
            if (!flag_d)
                handle_elem(&state, &my_dirent);
            listdir(".", &state);
            if (flag_d)
                handle_elem(&state, &my_dirent);
        }
    }
    for(int j = start; j < end; j++)
    {
        struct stat buf;
        int x;
        if (symlink_flag > 0)
            x = stat (argv[j], &buf);
        else
            x = lstat (argv[j], &buf);
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
            handle_elem(&state, &my_dirent);
        }
        else
        {
            if (!flag_d)
                handle_elem(&state, &my_dirent);
            listdir(argv[j], &state);
            if (flag_d)
                handle_elem(&state, &my_dirent);
        }
    }
    free_expr(state.expr, &state);
}
