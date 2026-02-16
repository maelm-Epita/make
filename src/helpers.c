#include "helpers.h"

#include <ctype.h>
#include <stddef.h>

int contains(char *line, char c)
{
    for (size_t i = 0; line[i] != 0; i++)
    {
        if (line[i] == c)
        {
            return 1;
        }
    }
    return 0;
}

int is_blank_line(char *line)
{
    for (size_t i = 0; line[i] != 0; i++)
    {
        char c = line[i];
        if (!isblank(c) && line[i] != '\n')
        {
            return 0;
        }
    }
    return 1;
}

int is_blank_line_ignore_comment(char *line)
{
    for (size_t i = 0; line[i] != 0; i++)
    {
        char c = line[i];
        if (c == '#')
        {
            return 1;
        }
        else if (!isblank(c) && line[i] != '\n')
        {
            return 0;
        }
    }
    return 1;
}

char *trim_l(char *line)
{
    while (*line != 0 && isblank(*line))
    {
        line++;
    }
    return line;
}

char *trim_r(char *line)
{
    size_t last_char = 0;
    for (size_t i = 0; line[i] != 0; i++)
    {
        if (!isblank(line[i]))
        {
            last_char = i;
        }
    }
    line[last_char + 1] = 0;
    return line;
}

char *trim(char *line)
{
    return trim_r(trim_l(line));
}
