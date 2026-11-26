#include "../include/builtin.h"
#include "../include/alias_list.h"
#include "../include/bg_process_list.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

static const BuildInCommand builtins[] = {
    {"alias",   cmd_alias},
    {"unalias", cmd_unalias},
    {"exit",    cmd_exit},
    {"fg",      cmd_fg},
    {NULL,      NULL}
};

// find finished processes and (soft) remove from list
static void reap_bg_processes(void)
{
    int status;
    pid_t ended_pid;
    while ((ended_pid = waitpid(-1, &status, WNOHANG)) > 0) {
        remove_bg_process_by_pid((int)ended_pid);
    }
}

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

    reap_bg_processes();

    if (has_active_bg_process()) {
        fprintf(stderr, "exit: there are background processes still running.\n");
        return -1;
    }

    free_alias_list();
    free_bg_process_list();
    exit(0);
}

int cmd_fg(int argc, char *argv[])
{
    if (argc != 2 || argv[1][0] != '%' || argv[1][1] == '\0') {
        fprintf(stderr, "Usage: fg %%<pid>\n");
        return -1;
    }

    char* endptr = NULL;
    long pid = strtol(argv[1] + 1, &endptr, 10);
    if (*endptr != '\0' || pid <= 0) {
        fprintf(stderr, "fg: invalid process id '%s'\n", argv[1]);
        return -1;
    }

    reap_bg_processes();

    if (find_bg_process_by_pid((int)pid) == NULL) {
        fprintf(stderr, "fg: no such background process %ld\n", pid);
        return -1;
    }

    remove_bg_process_by_pid((int)pid);

    int status;
    while (waitpid((pid_t)pid, &status, 0) == -1) {
        if (errno != EINTR) {
            perror("fg: waitpid");
            return -1;
        }
    }

    return 0;
}