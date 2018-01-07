#include <err.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ptrace.h>

#include "dbg.h"

struct bp_list *bplist_init(void)
{
    struct bp_list *list = malloc(sizeof(struct bp_list));
    if (!list)
        return NULL;
    list->size = 0;
    list->id = 1;
    list->head = NULL;
    return list;
}

void bplist_add(struct bplist_node *node)
{
    node->next = g_ctx.bp_list->head;
    g_ctx.bp_list->head = node;
    ++g_ctx.bp_list->size;
}

void bplist_remove(long id)
{
    struct bplist_node *prev = NULL;
    int found = 0;
    struct breakpoint *bp;
    for (struct bplist_node *tmp = g_ctx.bp_list->head; tmp;) {
        bp = CONTAINER_OF(struct breakpoint, node, tmp);
        if (bp->id == id) {
            found = 1;
            --g_ctx.bp_list->size;
            if (prev)
                prev->next = tmp->next;
            else
                g_ctx.bp_list->head = tmp->next;
            if (ptrace(PTRACE_POKETEXT, g_ctx.child_pid, (void *)bp->addr,
                (void *)bp->content) == -1) {
                printf("Cannot modify data at address 0x%lx\n", bp->addr);
            }
            free(bp);
            tmp = NULL;
        }
        else
            prev = tmp;
        if (tmp)
            tmp = tmp->next;
    }
    if (!found)
        printf("No breakpoint number %lu\n", id);
}

void bplist_destroy(void)
{
    for (struct bplist_node *tmp = g_ctx.bp_list->head; tmp;) {
        struct breakpoint *bp = CONTAINER_OF(struct breakpoint, node, tmp);
        tmp = tmp->next;
        free(bp);
    }
    free(g_ctx.bp_list);
}

struct breakpoint *get_breakpoint(long addr)
{
    for (struct bplist_node *tmp = g_ctx.bp_list->head; tmp; tmp = tmp->next) {
        struct breakpoint *bp = CONTAINER_OF(struct breakpoint, node, tmp);
        if (bp->addr == addr)
            return bp;
    }
    return NULL;
}
