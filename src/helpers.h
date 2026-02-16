#ifndef HELPERS_H
#define HELPERS_H

int contains(char *line, char c);

int is_blank_line(char *line);

int is_blank_line_ignore_comment(char *line);

char *trim_l(char *line);

char *trim_r(char *line);

char *trim(char *line);

#endif /* HELPERS_H */
