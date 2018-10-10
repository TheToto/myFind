#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <err.h>
#include <stdlib.h>
#include "my_string.h"
#include "commands.h"

#define NB_FUNC 2

struct test tests[NB_FUNC] =
{
    { "-name", &t_name },
    { "-print", &a_print}
};


int handle_elem(struct state *state, struct my_dirent *my_dirent)
{
    /*for (int i = 0; i < state->funcs_len; i++)
    {
        if (!state->funcs[i].func(my_dirent, state->funcs[i].arg))
            return 0;
    }*/
    (void)state;
    printf("%s\n", my_dirent->path);
    return 1;
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

            struct my_dirent my_dirent = { dp->d_name, new_path, &buf };

            if (!state->flag_d)
                handle_elem(state, &my_dirent);

            if (S_ISDIR(buf.st_mode))
                listdir(new_path, state);

            if (state->flag_d)
                handle_elem(state, &my_dirent);
        }
    }
    closedir(dir);
    return 0;
}

struct expr *parse_expr(int *i, char **argv, int argc)
{
    fprintf(stderr, "d: Enter un (\n");
    struct expr *expr = malloc(sizeof(struct expr));
    expr->expr = calloc(argc * 2, sizeof(struct expr*));
    expr->func = NULL;
    int line = 0;
    int col = 0;
    for (; *i < argc; (*i)++)
    {
        if (my_strcmp(argv[*i], "(") == 0)
        {
            (*i)++;
            expr->expr[line*argc+col] = parse_expr(i, argv, argc);
            col++;
            continue;
        }
        if (my_strcmp(argv[*i], ")") == 0)
        {
            fprintf(stderr, "d: Close )\n");
            (*i)++;
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
        int k = 0;
        for(; k < NB_FUNC; k++)
        {
            if (my_strcmp(tests[k].name, argv[*i]) == 0)
            {
                char *arg = NULL;
                if (argv[*i+1] && argv[*i+1][0] != '-'
                        && argv[*i+1][0] != '('
                        && argv[*i+1][0] != ')')
                {
                    arg = argv[*i+1];
                    (*i)++;
                }
                struct func *func = malloc(sizeof(struct func));
                func->func = tests[k].func;
                func->arg = arg;
                fprintf(stderr, "d: Add %s with %s\n", tests[k].name, arg);
                struct expr *new = malloc(sizeof(struct expr));
                new->expr = NULL;
                new->func = func;
                expr->expr[line*argc+col] = new;
                col++;
                break;
            }
        }
        if (k >= NB_FUNC)
            errx(1, "cannot do parsing expr: unknown %s",argv[*i]);
    }
    return expr;
}

int main(int argc, char **argv)
{
    int i = 1;
    int flag_d = 0;
    int symlink_flag = 0;
    for(; i < argc; i++)
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
            errx(1, "cannot do parsing options: unknown %s",argv[i]);
        }
    }
    int start = i;
    for (; i < argc; i++)
    {
        if (argv[i][0] == '-')
            break;
    }
    int end = i;

    struct expr *expr = parse_expr(&i , argv, argc);
    struct state state = { expr, flag_d, symlink_flag };

    // Launch files
    if (start == end)
    {
        listdir(".", &state);
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
        struct my_dirent my_dirent = { argv[j], argv[j], &buf };
        if (!S_ISDIR(buf.st_mode))
        {
            printf("%s\n", argv[j]);
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
}
