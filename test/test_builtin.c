#include "../include/builtin.h"
#include "../include/alias_list.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_find_builtin_fn(void)
{
    assert(find_builtin_fn("alias") == cmd_alias);
    assert(find_builtin_fn("unalias") == cmd_unalias);
    assert(find_builtin_fn("exit") == cmd_exit);
    assert(find_builtin_fn("fg") == cmd_fg);

    assert(find_builtin_fn("ls") == NULL);
    assert(find_builtin_fn("") == NULL);
}

static void test_cmd_alias(void)
{
    free_alias_list();

    // add
    char* argv_add[] = {"alias", "ls -l", "list", NULL};
    assert(cmd_alias(3, argv_add) == 0);
    const char* cmd = get_command_by_name("list");
    assert(cmd != NULL && strcmp(cmd, "ls -l") == 0);

    // list
    char* argv_list[] = {"alias", "-l", NULL};
    assert(cmd_alias(2, argv_list) == 0);

    // invalid uses
    char* argv_no_args[] = {"alias", NULL};
    assert(cmd_alias(1, argv_no_args) == -1);

    char* argv_bad_flag[] = {"alias", "-x", NULL};
    assert(cmd_alias(2, argv_bad_flag) == -1);

    char* argv_too_many[] = {"alias", "ls", "-l", "list", NULL};
    assert(cmd_alias(4, argv_too_many) == -1);

    free_alias_list();
}

static void test_cmd_unalias(void)
{
    free_alias_list();

    char* argv_add[] = {"alias", "ps -a", "proc", NULL};
    assert(cmd_alias(3, argv_add) == 0);

    //unalias
    char* argv_rm[] = {"unalias", "proc", NULL};
    assert(cmd_unalias(2, argv_rm) == 0);
    assert(get_command_by_name("proc") == NULL);

    // delete unexisting
    assert(cmd_unalias(2, argv_rm) == -1);

    // invalid arg count
    char* argv_no_args[] = {"unalias", NULL};
    assert(cmd_unalias(1, argv_no_args) == -1);

    char* argv_too_many[] = {"unalias", "a", "b", NULL};
    assert(cmd_unalias(3, argv_too_many) == -1);

    free_alias_list();
}

static void test_cmd_exit_and_fg_stubs(void)
{
    char* argv_exit[] = {"exit", NULL};
    assert(cmd_exit(1, argv_exit) == 0);

    char* argv_fg[] = {"fg", "%1", NULL};
    assert(cmd_fg(2, argv_fg) == 0);
}

int main(void)
{
    test_find_builtin_fn();
    test_cmd_alias();
    test_cmd_unalias();
    test_cmd_exit_and_fg_stubs();

    printf("[PASS] All builtin tests passed.\n");
    return 0;
}