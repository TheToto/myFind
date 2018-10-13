#ifndef PARSING_H
#define PARSING_H

#include "commands.h"

#define NB_FUNC 10

void parse_flag(int *i, struct state *state, int argc, char **argv);
struct expr *parse_expr(int *i, char **argv, int argc, struct state *state);
void free_expr(struct expr *expr, struct state *state);
int make_stat(struct stat *buf, struct state *state, char *arg);

#endif /* !PARSING_H */