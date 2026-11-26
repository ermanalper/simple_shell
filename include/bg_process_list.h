#pragma once

typedef struct BgProcessNode {
    struct BgProcessNode* next;
    int pid;
    int is_still_bg; // use this after termination or moving to fg
} BgProcessNode;

int add_bg_process(int pid);
BgProcessNode* find_bg_process_by_pid(int pid);
int remove_bg_process_by_pid(int pid);
int has_active_bg_process(void);
void free_bg_process_list(void);