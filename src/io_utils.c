#include "./io_utils.h"
#include <string.h>
#include <stdlib.h>

int write_double(FILE *f, double d)
{
    int w = fwrite(&d, sizeof(d), 1, f);
    return w == sizeof(d);
}

int read_double(FILE *f, double *d)
{
    int r = fread(d, sizeof * d, 1, f);
    return r == sizeof * d;
}

int write_int(FILE *f, int i)
{
    int w = fwrite(&i, sizeof(i), 1, f);
    return w == sizeof(i);
}

int read_int(FILE *f, int *i)
{
    int r = fread(i, sizeof * i, 1, f);
    return r == sizeof * i;
}

int write_size_t(FILE *f, size_t i)
{
    int w = fwrite(&i, sizeof(i), 1, f);
    return w == sizeof(i);
}

int read_size_t(FILE *f, size_t *i)
{
    int r = fread(i, sizeof * i, 1, f);
    return r == sizeof * i;
}

int write_str(FILE *f, char *str)
{
    size_t len = strlen(str);
    if (!write_size_t(f, len)) return 0;

    int w = fwrite(str, sizeof * str, len, f);
    return w == len * sizeof * str;
}

int read_str(FILE *f, char **str)
{
    size_t len;
    if (!read_size_t(f, &len)) return 0;

    *str = malloc(sizeof ** str * (len + 1));
    if (str == NULL) return 0;
    *str[len] = 0;

    int r = fread(*str, sizeof ** str, len, f);
    return r == sizeof ** str * len;
}

