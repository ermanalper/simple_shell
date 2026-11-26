#include "../include/builtin.h"
#include "../include/alias_list.h"
#include "../include/bg_process_list.h"

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// fork a child that sleeps for `ms` milliseconds and then exits with `code`
static pid_t spawn_sleeper(int ms, int code)
{
    fflush(NULL);
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
        usleep((useconds_t)ms * 1000);
        _exit(code);
    }
    return pid;
}

static void make_fg_arg(char* buf, size_t size, pid_t pid)
{
    snprintf(buf, size, "%%%d", (int)pid);
}

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

    // unalias
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

/* ---------------- exit ---------------- */

// cmd_exit terminates the process, so it is run inside a forked "shell" child
static void test_cmd_exit_no_bg(void)
{
    fflush(NULL);
    pid_t shell = fork();
    assert(shell >= 0);
    if (shell == 0) {
        free_bg_process_list();
        add_alias("list", "ls -l");
        char* argv_exit[] = {"exit", NULL};
        cmd_exit(1, argv_exit);
        _exit(1); // must not be reached
    }

    int status;
    assert(waitpid(shell, &status, 0) == shell);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static void test_cmd_exit_with_running_bg(void)
{
    free_bg_process_list();

    pid_t bg = spawn_sleeper(2000, 0);
    assert(add_bg_process(bg) == 0);

    // refuses to exit and returns
    char* argv_exit[] = {"exit", NULL};
    assert(cmd_exit(1, argv_exit) == -1);
    assert(find_bg_process_by_pid(bg) != NULL);

    kill(bg, SIGKILL);
    waitpid(bg, NULL, 0);
    free_bg_process_list();
}

static void test_cmd_exit_after_bg_finished(void)
{
    // the shell child owns the bg process so that cmd_exit can reap it
    fflush(NULL);
    pid_t shell = fork();
    assert(shell >= 0);
    if (shell == 0) {
        free_bg_process_list();
        pid_t bg = spawn_sleeper(0, 0);
        add_bg_process(bg);
        usleep(200 * 1000); // let bg become a zombie
        char* argv_exit[] = {"exit", NULL};
        cmd_exit(1, argv_exit);
        _exit(1); // reached only if exit refused
    }

    int status;
    assert(waitpid(shell, &status, 0) == shell);
    assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/* ---------------- fg ---------------- */

static void test_cmd_fg_invalid_args(void)
{
    free_bg_process_list();

    char* argv_none[] = {"fg", NULL};
    assert(cmd_fg(1, argv_none) == -1);

    char* argv_too_many[] = {"fg", "%1", "%2", NULL};
    assert(cmd_fg(3, argv_too_many) == -1);

    char* argv_no_percent[] = {"fg", "123", NULL};
    assert(cmd_fg(2, argv_no_percent) == -1);

    char* argv_only_percent[] = {"fg", "%", NULL};
    assert(cmd_fg(2, argv_only_percent) == -1);

    char* argv_not_number[] = {"fg", "%abc", NULL};
    assert(cmd_fg(2, argv_not_number) == -1);

    char* argv_trailing[] = {"fg", "%12x", NULL};
    assert(cmd_fg(2, argv_trailing) == -1);

    char* argv_zero[] = {"fg", "%0", NULL};
    assert(cmd_fg(2, argv_zero) == -1);

    char* argv_negative[] = {"fg", "%-5", NULL};
    assert(cmd_fg(2, argv_negative) == -1);
}

static void test_cmd_fg_unknown_pid(void)
{
    free_bg_process_list();

    char* argv_fg[] = {"fg", "%999999", NULL};
    assert(cmd_fg(2, argv_fg) == -1);
}

static void test_cmd_fg_running_process(void)
{
    free_bg_process_list();

    pid_t bg = spawn_sleeper(300, 7);
    assert(add_bg_process(bg) == 0);

    char arg[32];
    make_fg_arg(arg, sizeof(arg), bg);
    char* argv_fg[] = {"fg", arg, NULL};

    // blocks until the process finishes
    assert(cmd_fg(2, argv_fg) == 0);

    // no longer a background process
    assert(find_bg_process_by_pid(bg) == NULL);
    assert(has_active_bg_process() == 0);

    // child has already been reaped by fg
    assert(waitpid(bg, NULL, WNOHANG) == -1);

    // second fg on the same pid fails
    assert(cmd_fg(2, argv_fg) == -1);

    free_bg_process_list();
}

static void test_cmd_fg_keeps_other_bg(void)
{
    free_bg_process_list();

    pid_t bg1 = spawn_sleeper(200, 0);
    pid_t bg2 = spawn_sleeper(2000, 0);
    assert(add_bg_process(bg1) == 0);
    assert(add_bg_process(bg2) == 0);

    char arg[32];
    make_fg_arg(arg, sizeof(arg), bg1);
    char* argv_fg[] = {"fg", arg, NULL};
    assert(cmd_fg(2, argv_fg) == 0);

    assert(find_bg_process_by_pid(bg1) == NULL);
    assert(find_bg_process_by_pid(bg2) != NULL);
    assert(has_active_bg_process() == 1);

    kill(bg2, SIGKILL);
    waitpid(bg2, NULL, 0);
    free_bg_process_list();
}

static void test_cmd_fg_already_finished(void)
{
    free_bg_process_list();

    pid_t bg = spawn_sleeper(0, 0);
    assert(add_bg_process(bg) == 0);
    usleep(200 * 1000); // let bg become a zombie

    // reaped before lookup, so it is no longer a bg process
    char arg[32];
    make_fg_arg(arg, sizeof(arg), bg);
    char* argv_fg[] = {"fg", arg, NULL};
    assert(cmd_fg(2, argv_fg) == -1);
    assert(find_bg_process_by_pid(bg) == NULL);

    free_bg_process_list();
}

int main(void)
{
    test_find_builtin_fn();
    test_cmd_alias();
    test_cmd_unalias();

    test_cmd_exit_no_bg();
    test_cmd_exit_with_running_bg();
    test_cmd_exit_after_bg_finished();

    test_cmd_fg_invalid_args();
    test_cmd_fg_unknown_pid();
    test_cmd_fg_running_process();
    test_cmd_fg_keeps_other_bg();
    test_cmd_fg_already_finished();

    free_alias_list();
    free_bg_process_list();
    printf("[PASS] All builtin tests passed.\n");
    return 0;
}
