#pragma once

typedef int (*builtin_fn_t) (int argc, char *argv[]);
typedef struct BuildInCommand {
    char* name;
    builtin_fn_t fn;
} BuildInCommand;

static builtin_fn_t find_builtin_fn(char* name);
static int cmd_alias(int argc, char *argv[]);
static int cmd_unalias(int argc, char *argv[]);
static int cmd_exit(int argc, char *argv[]);
static int cmd_fg(int argc, char *argv[]);

static const BuildInCommand builtins[] = {
    {"alias", cmd_alias},
    {"unalias", cmd_unalias},
    {"exit", cmd_exit},
    {"fg", cmd_fg}
};