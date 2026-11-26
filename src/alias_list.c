#include "../include/alias_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AliasNode* head = NULL;
static AliasNode* tail = NULL;

static char* dup_str(const char* src)
{
    size_t len = strlen(src) + 1;
    char* copy = (char*) malloc(len);
    if (copy) {
        memcpy(copy, src, len);
    }
    return copy;
}

static AliasNode* get_alias_by_name(const char* name, int include_deleted)
{
    for (AliasNode* cur = head; cur != NULL; cur = cur->next) {
        if ((include_deleted || !cur->is_deleted) && strcmp(cur->name, name) == 0) {
            return cur;
        }
    }
    return NULL;
}

int add_alias(const char* name, const char* command)
{
    if (!name || !command) return -1;

    // override the previously deleted alias
    AliasNode* existing = get_alias_by_name(name, 1);
    if (existing) {
        char* new_cmd = dup_str(command);
        if (!new_cmd) {
            fprintf(stderr, "Could not allocate space for alias command\n");
            return -1;
        }
        free(existing->command);
        existing->command = new_cmd;
        existing->is_deleted = 0;
        return 0;
    }

    AliasNode* new_alias = (AliasNode*) malloc(sizeof(AliasNode));
    if (!new_alias) {
        fprintf(stderr, "Could not allocate space for new alias\n");
        return -1;
    }

    new_alias->name = dup_str(name);
    new_alias->command = dup_str(command);
    if (!new_alias->name || !new_alias->command) {
        fprintf(stderr, "Could not allocate space for alias strings\n");
        free(new_alias->name);
        free(new_alias->command);
        free(new_alias);
        return -1;
    }

    new_alias->is_deleted = 0;
    new_alias->next = NULL;

    if (head == NULL) {
        head = tail = new_alias;
    } else {
        tail->next = new_alias;
        tail = new_alias;
    }
    return 0;
}

const char* get_command_by_name(const char* name)
{
    AliasNode* alias = get_alias_by_name(name, 0);
    return alias ? alias->command : NULL;
}

int remove_alias_by_name(const char* name)
{
    AliasNode* alias = get_alias_by_name(name, 0);
    if (!alias) {
        fprintf(stderr, "unalias: could not find alias '%s'\n", name);
        return -1;
    }
    alias->is_deleted = 1;
    return 0;
}

void print_aliases(void)
{
    for (AliasNode* cur = head; cur != NULL; cur = cur->next) {
        if (!cur->is_deleted) {
            printf("\t%s \"%s\"\n", cur->name, cur->command);
        }
    }
}

void free_alias_list(void)
{
    AliasNode* current = head;
    while (current != NULL) {
        AliasNode* to_be_deleted = current;
        current = current->next;
        free(to_be_deleted->name);
        free(to_be_deleted->command);
        free(to_be_deleted);
    }
    head = NULL;
    tail = NULL;
}