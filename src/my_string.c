#include <stddef.h>
#include <stdlib.h>

size_t my_strlen(const char *s)
{
  size_t l = 0;
  for (; s[l] != '\0'; l++)
    continue;
  return l;
}

char *my_strcpy(const char *s, char *d)
{
  size_t i = 0;
  for (; s[i] != '\0'; i++)
    d[i] = s[i];
  d[i] = '\0';
  return d;
}

char *my_strcat(char *dest, const char *src)
{
  size_t srcl = my_strlen(src);
  size_t destl = my_strlen(dest);
  for (size_t i = 0; i < srcl; i++)
  {
    dest[destl + i] = src[i];
  }
  dest[srcl + destl] = '\0';
  return dest;
}

int my_strcmp(const char *s1, const char *s2)
{
  size_t i = 0;
  for (; s1[i] != '\0' && s2[i] != '\0'; i++)
    if (s1[i] != s2[i])
      return 1;
  if (s1[i] == s2[i])
    return 0;
  return 1;
}

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
