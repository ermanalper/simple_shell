#include "../include/builtin.h"
#include "../include/alias_list.h"

#include <stdio.h>
#include <string.h>

static const BuildInCommand builtins[] = {
    {"alias",   cmd_alias},
    {"unalias", cmd_unalias},
    {"exit",    cmd_exit},
    {"fg",      cmd_fg},
    {NULL,      NULL}
};

builtin_fn_t find_builtin_fn(const char* name)
{
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(builtins[i].name, name) == 0) {
            return builtins[i].fn;
        }
    }
    return NULL;
}

int cmd_alias(int argc, char *argv[])
{
    /* alias -l */
    if (argc == 2 && strcmp(argv[1], "-l") == 0) {
        print_aliases();
        return 0;
    }

    // add alias
    if (argc == 3) {
        const char* command = argv[1];
        const char* name = argv[2];
        return add_alias(name, command);
    }

    fprintf(stderr, "Usage: alias -l | alias \"<command>\" <name>\n");
    return -1;
}

int cmd_unalias(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: unalias <name>\n");
        return -1;
    }
    return remove_alias_by_name(argv[1]);
}

int cmd_exit(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    return 0;
}

int cmd_fg(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    return 0;
}