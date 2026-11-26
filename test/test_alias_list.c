#include "../include/alias_list.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_empty_list(void)
{
    free_alias_list();
    assert(get_command_by_name("list") == NULL);
    assert(remove_alias_by_name("list") == -1);
    print_aliases(); // assert no crash
    free_alias_list();
}

static void test_add_and_get(void)
{
    free_alias_list();

    assert(add_alias(NULL, "ls -l") == -1);
    assert(add_alias("list", NULL) == -1);
    assert(add_alias(NULL, NULL) == -1);

    assert(add_alias("list", "ls -l") == 0);
    assert(add_alias("proc", "ps -a") == 0);

    const char* cmd1 = get_command_by_name("list");
    const char* cmd2 = get_command_by_name("proc");
    assert(cmd1 != NULL && strcmp(cmd1, "ls -l") == 0);
    assert(cmd2 != NULL && strcmp(cmd2, "ps -a") == 0);
    assert(get_command_by_name("unknown") == NULL);

    free_alias_list();
}

static void test_deep_copy(void)
{
    free_alias_list();

    char name_buf[32] = "ll";
    char cmd_buf[32] = "ls -la";

    assert(add_alias(name_buf, cmd_buf) == 0);

    // deep copy
    strcpy(name_buf, "changed");
    strcpy(cmd_buf, "rm -rf");

    const char* stored = get_command_by_name("ll");
    assert(stored != NULL && strcmp(stored, "ls -la") == 0);

    free_alias_list();
}

static void test_overwrite_active_alias(void)
{
    free_alias_list();

    assert(add_alias("list", "ls -l") == 0);
    assert(add_alias("list", "ls -la") == 0);

    const char* cmd = get_command_by_name("list");
    assert(cmd != NULL && strcmp(cmd, "ls -la") == 0);

    free_alias_list();
}

static void test_remove_and_readd(void)
{
    free_alias_list();

    assert(add_alias("list", "ls -l") == 0);
    assert(remove_alias_by_name("list") == 0);

    // previously deleted cannot be deleted again
    assert(get_command_by_name("list") == NULL);
    assert(remove_alias_by_name("list") == -1);

    // previously deleted can be added
    assert(add_alias("list", "ls -lh") == 0);
    const char* cmd = get_command_by_name("list");
    assert(cmd != NULL && strcmp(cmd, "ls -lh") == 0);

    free_alias_list();
}

static void test_free_and_reuse(void)
{
    free_alias_list();

    assert(add_alias("a", "echo a") == 0);
    free_alias_list();

    assert(get_command_by_name("a") == NULL);
    assert(add_alias("b", "echo b") == 0);
    assert(strcmp(get_command_by_name("b"), "echo b") == 0);

    free_alias_list();
}

int main(void)
{
    test_empty_list();
    test_add_and_get();
    test_deep_copy();
    test_overwrite_active_alias();
    test_remove_and_readd();
    test_free_and_reuse();

    printf("[PASS] All alias_list tests passed.\n");
    return 0;
}