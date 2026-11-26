#pragma once

typedef int (*builtin_fn_t)(int argc, char *argv[]);

typedef struct BuildInCommand {
    const char* name;
    builtin_fn_t fn;
} BuildInCommand;

builtin_fn_t find_builtin_fn(const char* name);
int cmd_alias(int argc, char *argv[]);
int cmd_unalias(int argc, char *argv[]);
int cmd_exit(int argc, char *argv[]);
int cmd_fg(int argc, char *argv[]);