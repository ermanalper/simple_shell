#include "../include/bg_process_list.h"

#include <assert.h>
#include <stdio.h>

static void test_empty_list(void)
{
    free_bg_process_list();
    assert(find_bg_process_by_pid(100) == NULL);
    assert(remove_bg_process_by_pid(100) == -1);
    assert(has_active_bg_process() == 0);
    free_bg_process_list();
}

static void test_invalid_pid(void)
{
    free_bg_process_list();
    assert(add_bg_process(0) == -1);
    assert(add_bg_process(-5) == -1);
    assert(has_active_bg_process() == 0);
    free_bg_process_list();
}

static void test_add_and_find(void)
{
    free_bg_process_list();

    assert(add_bg_process(100) == 0);
    assert(add_bg_process(200) == 0);
    assert(add_bg_process(300) == 0);

    BgProcessNode* n1 = find_bg_process_by_pid(100);
    BgProcessNode* n2 = find_bg_process_by_pid(200);
    BgProcessNode* n3 = find_bg_process_by_pid(300);
    assert(n1 != NULL && n1->pid == 100 && n1->is_still_bg == 1);
    assert(n2 != NULL && n2->pid == 200 && n2->is_still_bg == 1);
    assert(n3 != NULL && n3->pid == 300 && n3->is_still_bg == 1);
    assert(find_bg_process_by_pid(400) == NULL);

    // insertion order (append to tail)
    assert(n1->next == n2 && n2->next == n3 && n3->next == NULL);

    assert(has_active_bg_process() == 1);
    free_bg_process_list();
}

static void test_add_duplicate(void)
{
    free_bg_process_list();

    assert(add_bg_process(100) == 0);
    assert(add_bg_process(100) == 0);

    BgProcessNode* n = find_bg_process_by_pid(100);
    assert(n != NULL && n->next == NULL); // no second node created

    free_bg_process_list();
}

static void test_remove(void)
{
    free_bg_process_list();

    assert(add_bg_process(100) == 0);
    assert(add_bg_process(200) == 0);

    assert(remove_bg_process_by_pid(100) == 0);
    assert(find_bg_process_by_pid(100) == NULL);
    assert(find_bg_process_by_pid(200) != NULL);
    assert(has_active_bg_process() == 1);

    // already removed cannot be removed again
    assert(remove_bg_process_by_pid(100) == -1);
    // never added
    assert(remove_bg_process_by_pid(999) == -1);

    assert(remove_bg_process_by_pid(200) == 0);
    assert(has_active_bg_process() == 0);

    free_bg_process_list();
}

static void test_remove_and_readd(void)
{
    free_bg_process_list();

    assert(add_bg_process(100) == 0);
    assert(add_bg_process(200) == 0);
    assert(remove_bg_process_by_pid(100) == 0);

    // pid reuse: soft-deleted node is reactivated, not duplicated
    assert(add_bg_process(100) == 0);
    BgProcessNode* n = find_bg_process_by_pid(100);
    assert(n != NULL && n->is_still_bg == 1);
    assert(n->next != NULL && n->next->pid == 200 && n->next->next == NULL);

    free_bg_process_list();
}

static void test_free_and_reuse(void)
{
    free_bg_process_list();

    assert(add_bg_process(100) == 0);
    free_bg_process_list();

    assert(find_bg_process_by_pid(100) == NULL);
    assert(has_active_bg_process() == 0);

    // tail must be reset as well, otherwise this would write to freed memory
    assert(add_bg_process(200) == 0);
    assert(add_bg_process(300) == 0);
    BgProcessNode* n = find_bg_process_by_pid(200);
    assert(n != NULL && n->next != NULL && n->next->pid == 300);

    // double free must be safe
    free_bg_process_list();
    free_bg_process_list();
}

int main(void)
{
    test_empty_list();
    test_invalid_pid();
    test_add_and_find();
    test_add_duplicate();
    test_remove();
    test_remove_and_readd();
    test_free_and_reuse();

    printf("[PASS] All bg_process_list tests passed.\n");
    return 0;
}
