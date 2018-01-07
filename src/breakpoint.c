#include <sys/ptrace.h>
#include <stdlib.h>

#include "breakpoint.h"
#include "dbg.h"

int add_breakpoint(long addr, enum bp_type type)
{
    long data = ptrace(PTRACE_PEEKTEXT, g_ctx.child_pid, (void *)addr, 0);
    if (data == -1)
        return 0;
    long trap = (data & 0xFFFFFF00) | 0xCC;
    if (ptrace(PTRACE_POKETEXT, g_ctx.child_pid, (void *)addr,
        (void *)trap) == -1)
        return 0;
    struct breakpoint bp;
    bp.addr = addr;
    bp.content = data;
    bp.type = type;
    bp.id = g_ctx.bp_list->id;
    ++g_ctx.bp_list->id;
    bplist_add(&bp.node);
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
