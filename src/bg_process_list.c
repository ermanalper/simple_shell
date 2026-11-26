#include "../include/bg_process_list.h"

#include <stdio.h>
#include <stdlib.h>

static BgProcessNode* head = NULL;
static BgProcessNode* tail = NULL;

static BgProcessNode* get_node_by_pid(int pid, int include_inactive)
{
    for (BgProcessNode* cur = head; cur != NULL; cur = cur->next) {
        if ((include_inactive || cur->is_still_bg) && cur->pid == pid) {
            return cur;
        }
    }
    return NULL;
}

int add_bg_process(int pid)
{
    if (pid <= 0) {
        return -1;
    }

    // add previously removed (soft delete) process
    BgProcessNode* existing = get_node_by_pid(pid, 1);
    if (existing) {
        existing->is_still_bg = 1;
        return 0;
    }

    BgProcessNode* new_node = (BgProcessNode*) malloc(sizeof(BgProcessNode));
    if (!new_node) {
        fprintf(stderr, "Could not allocate space for new background process\n");
        return -1;
    }

    new_node->pid = pid;
    new_node->is_still_bg = 1;
    new_node->next = NULL;

    if (head == NULL) {
        head = tail = new_node;
    } else {
        tail->next = new_node;
        tail = new_node;
    }
    return 0;
}

BgProcessNode* find_bg_process_by_pid(int pid)
{
    return get_node_by_pid(pid, 0);
}

int remove_bg_process_by_pid(int pid)
{
    BgProcessNode* node = get_node_by_pid(pid, 0);
    if (!node) {
        return -1;
    }
    node->is_still_bg = 0;
    return 0;
}

int has_active_bg_process(void)
{
    for (BgProcessNode* cur = head; cur != NULL; cur = cur->next) {
        if (cur->is_still_bg) {
            return 1;
        }
    }
    return 0;
}

void free_bg_process_list(void)
{
    BgProcessNode* current = head;
    while (current != NULL) {
        BgProcessNode* to_be_deleted = current;
        current = current->next;
        free(to_be_deleted);
    }
    head = NULL;
    tail = NULL;
}