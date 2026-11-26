#pragma once

typedef struct AliasNode {
    struct AliasNode* next;
    char* name;
    char* command;
    int is_deleted;
} AliasNode;

int add_alias(const char* name, const char* command);
int remove_alias_by_name(const char* name);
const char* get_command_by_name(const char* name);
void print_aliases(void);
void free_alias_list(void);