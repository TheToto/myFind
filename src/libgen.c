#include <stddef.h>
#include "my_string.h"

char *my_dirname (const char *path, char *res)
{
    my_strcpy(path, res);
    size_t len = my_strlen(path);
    size_t i = len - 2;
    while ('/' != res[i] && i != 0 && i < len )
        i--;
    if (res[i] == '/')
        res[i] = '\0';
    return res;
}

char *my_filename (const char *path, char *res)
{
    size_t len = my_strlen(path);
    size_t i = len - 2;
    while ('/' != path[i] && i != 0 && i < len )
        i--;
    if (path[i] == '/')
        my_strcpy(path+i+1, res);
    else
        my_strcpy(path, res);
    return res;
}
