#include <sys/ptrace.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "breakpoint.h"
#include "dbg.h"

static int add_breakpoint(long addr, enum bp_type type)
{
    long data = ptrace(PTRACE_PEEKDATA, g_ctx.child_pid,
                       (void *)addr, NULL);
    if (data == -1) {
        printf("Cannot access data at address 0x%lx\n", addr);
        return 0;
    }
    long trap = (data & ~(0xFF)) | 0xCC;
    if (ptrace(PTRACE_POKETEXT, g_ctx.child_pid, (void *)addr,
               (void *)trap) == -1) {
        printf("Cannot modify data at address 0x%lx\n", addr);
        return 0;
    }
    struct breakpoint *bp = malloc(sizeof(struct breakpoint));
    if (!bp)
        return 0;
    bp->addr = addr;
    bp->content = data;
    bp->type = type;
    bp->id = g_ctx.bp_list->id;
    printf("Breakpoint %ld added at address 0x%lx\n", bp->id, addr); 
    ++g_ctx.bp_list->id;
    bplist_add(&bp->node);
    return 1;
}

static void print_bplist(struct bplist_node *node)
{
    if (node) {
        print_bplist(node->next);
        struct breakpoint *bp = CONTAINER_OF(struct breakpoint, node, node);
        if (bp->type == BREAK)
            printf("Breakpoint : ");
        else
            printf("Temporary breakpoint : ");
        printf("id = %ld addr = 0x%lx\n", bp->id, bp->addr);
    }
}

int do_break_list(void *UNUSED(args))
{
    print_bplist(g_ctx.bp_list->head);
    return 1;
}

int do_tbreak(void *args)
{
    if (!args)
        return 0;
    long addr = (long)strtol((char *)args, NULL, 0);
    add_breakpoint(addr, TBREAK);
    return 1;
}

int do_break(void *args)
{
    if (!args)
        return 0;
    long addr = (long)strtol((char *)args, NULL, 0);
    add_breakpoint(addr, BREAK);
    return 1;
}
