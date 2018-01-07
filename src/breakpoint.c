#include <sys/ptrace.h>
#include <stdlib.h>

#include "breakpoint.h"
#include "dbg.h"

int addr_breakpoint(long addr)
{
    long data = ptrace(PTRACE_PEEKTEXT, g_ctx.child_pid, (void *)addr, 0);
    long trap = (data & 0xFFFFFF00) | 0xCC;
    if (ptrace(PTRACE_POKETEXT, g_ctx.child_pid, (void *)addr,
        (void *)trap) == -1)
        return 0;
    struct breakpoint bp;
    bp.addr = addr;
    bp.content = data;
    bp.id = g_ctx.bp_list->id;
    ++g_ctx.bp_list->id;
    bplist_add(&bp.node);
    return 1;
}

int do_break(void *args)
{
    if (!args)
        return 0;
    long addr = (long)strtol((char *)args, NULL, 0);
    addr_breakpoint(addr);
    return 1;
}
