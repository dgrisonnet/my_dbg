#define UNW_LOCAL_ONLY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <capstone/capstone.h>
#include <libunwind.h>

#include "cmd.h"
#include "dbg.h"

static int do_backtrace(void *UNUSED(args))
{
    unw_cursor_t cursor;
    unw_context_t uc;
    unw_word_t ip;
    unw_word_t sp;
    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);
    while (unw_step(&cursor) > 0) {
        unw_get_reg(&cursor, UNW_REG_IP, &ip);
        unw_get_reg(&cursor, UNW_REG_SP, &sp);
        printf("ip = %lx, sp = %lx\n", (long)ip, (long)sp);
    }
    return 1;
}

static void examine_assembly(char *data, size_t size, long addr)
{
    uint8_t *code = calloc(size, 1);
    memcpy(code, data, size);
    csh handle;
    cs_insn *insn;
    size_t count;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        free(code);
        return;
    }
    count = cs_disasm(handle, code, size, addr, 0, &insn);
    if (count > 0) {
        size_t j;
        for (j = 0; j < count; j++) {
            printf("0x%"PRIx64":\t%s\t\t%s\n", insn[j].address,
                   insn[j].mnemonic, insn[j].op_str);
        }
        cs_free(insn, count);
    }
    else 
        printf("Failed to dissasemble given code\n");
    cs_close(&handle);
    free(code);
}

static void examine_numbers(char *data, size_t size, size_t type,
                            long addr)
{
    int pretty_print = 16;
    for (size_t i = 0; i < size; i += type, pretty_print += type) {
        if (pretty_print == 16) {
            if (i)
                printf("\n");
            printf("0x%lx: ", addr + i);
            pretty_print = 0;
        }
        long tmp = 0;
        memcpy(&tmp, data + i, size - 1 - i < type ? size - 1 - i : type);
        if (type == 8)
            printf("0x%08lx ", tmp);
        else
            printf("%ld ", tmp);
    }
    printf("\n");
}

static int examine_print(char *data, char format, size_t size,
                         long addr)
{
    switch (format) {
    case 's':
        printf("0x%lx: %s\n", addr, data);
        break;
    case 'd':
        examine_numbers(data, size, 4, addr);
        break;
    case 'x':
        examine_numbers(data, size, 8, addr);
        break;
    case 'i':
        examine_assembly(data, size, addr);
        break;
    default:
        return 1;
    }
    return 0;
}

static int do_examine(void *args)
{
    char format;
    size_t size;
    long addr;
    if (sscanf((char *)args, "%c %zu %lx", &format, &size, &addr) == EOF)
        return 1;
    char *data = calloc(size + 1, 1);
    for (size_t i = 0; i < size; i += 4) {
        long tmp = ptrace(PTRACE_PEEKTEXT, g_ctx.child_pid,
                          (void *)(addr + i), NULL);
        if (tmp == -1) {
            free(data);
            return 1;
        }
        memcpy(data + i, &tmp, size - 1 - i < 4 ? size - 1 - i : 4);
    }
    int res = examine_print(data, format, size, addr);
    free(data);
    return res;
}

static int do_single_step(void *args)
{
    (void)args;
    if (ptrace(PTRACE_SINGLESTEP, g_ctx.child_pid, NULL, NULL) == -1)
        return 0;
    int status;
    siginfo_t siginfo;
    wait(&status);
    if (WIFSTOPPED(status)) {
        if (ptrace(PTRACE_GETSIGINFO, g_ctx.child_pid, NULL, &siginfo) == -1)
            return 0;
        if (siginfo.si_signo != SIGTRAP)
            return 0;
        printf("%s eip = 0x%lx\n", strsignal(WSTOPSIG(status)),
               (long)siginfo.si_addr);
    }
    else {
        printf("Process terminated\n");
        return 1;
    }
    return 0;
}

static int do_continue(void *UNUSED(args))
{
    struct user_regs_struct regs;
    if (ptrace(PTRACE_CONT, g_ctx.child_pid, NULL, NULL) == -1) { 
        printf("ptrace fail\n");
        return 0;
    }
    int status;
    wait(&status);
    if (WIFSTOPPED(status))
        printf("%s\n", strsignal(WSTOPSIG(status)));
    else {
        printf("Process terminated\n");
        return 1;
    }
    if (ptrace(PTRACE_GETREGS, g_ctx.child_pid, NULL, &regs) == -1) {
        printf("ptrace fail\n");
        return 0;
    }
    --regs.rip;
    struct breakpoint *bp = get_breakpoint(regs.rip);
    if (!bp)
        return 0;
    void *data = (void *)(bp->content);
    if (ptrace(PTRACE_POKETEXT, g_ctx.child_pid, (void *)regs.rip, data)
        == -1) {
        printf("Cannot modify data at address 0x%llx\n", regs.rip);
        return 0;
    }
    if (ptrace(PTRACE_SETREGS, g_ctx.child_pid, NULL, &regs) == -1) {
        printf("ptrace fail\n");
        return 0;
    }
    if (ptrace(PTRACE_SINGLESTEP, g_ctx.child_pid, NULL, NULL) == -1) {
        printf("ptrace fail\n");
        return 0;
    }
    if (bp->type != TBREAK) {
        long trap = (bp->content & 0xFFFFFF00) | 0xCC;
        if (ptrace(PTRACE_POKETEXT, g_ctx.child_pid, (void *)bp->addr,
                   (void *)trap) == -1)
            return 0; 
    }
    return 1;
}

static int is_call_instruction(long data, long addr, unsigned short *size)
{
    uint8_t *code = calloc(4, 1);
    memcpy(code, &data, 4);
    csh handle;
    cs_insn *insn;
    size_t count;
    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        free(code);
        return 0;
    }
    count = cs_disasm(handle, code, 4, addr, 0, &insn);
    int is_call = 0;
    if (count > 0) {
        if (insn[0].id == X86_INS_CALL) {
            *size = insn[0].size;
            count = 1;
        }
        cs_free(insn, count);
    }
    cs_close(&handle);
    free(code);
    return is_call;
}

static int do_next_instruction(void *UNUSED(args))
{
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, g_ctx.child_pid, NULL, &regs) == -1) {
        printf("ptrace fail\n");
        return 0;
    }
    long addr = regs.rip;
    long data = ptrace(PTRACE_PEEKDATA, g_ctx.child_pid,
                       (void *)addr, NULL);
    if (data == -1) {
        printf("Cannot access data at address 0x%lx\n", addr);
        return 0;
    }
    unsigned short size;
    if (!is_call_instruction(data, addr, &size))
        do_single_step(NULL);
    else {
        do_tbreak(&addr);
        do_continue(NULL);
    }
    return 1;
}

static int do_quit(void *UNUSED(args))
{
    return -1;
}

static int do_help(void *UNUSED(args))
{
    for (size_t i = 0; i < (size_t)(__stop_cmds - __start_cmds); ++i) {
        struct cmd *tmp = __start_cmds + i;
        printf("%s: %s\n", tmp->cmd, tmp->desc);
    }
    return 1;
}

int exec_cmd(char *cmd)
{
    int res = 1;
    for (size_t i = 0; i < (size_t)(__stop_cmds - __start_cmds); ++i) {
        struct cmd *tmp = __start_cmds + i;
        size_t len = strlen(tmp->cmd);
        if (!strncmp(cmd, tmp->cmd, len) && (cmd[len] == '\0' ||
            cmd[len] == ' ')) {
            char *args = cmd + strlen(tmp->cmd) + 1;
            res = (*tmp->fn)(args);
            break;
        }
    }
    return res;
}

shell_cmd(info_regs, display_registers, do_info_regs);
shell_cmd(continue, continue program being debugged, do_continue);
shell_cmd(quit, exit, do_quit);
shell_cmd(help, display this help message, do_help);
shell_cmd(info_memory, display_memory, do_info_memory);
shell_cmd(break, add a breakpoint, do_break);
shell_cmd(step_instr, single step the program being debugged, do_single_step);
shell_cmd(examine, print data at a specific address, do_examine);
shell_cmd(backtrace, printi the call trace at the current %rip, do_backtrace);
shell_cmd(tbreak, add a temporary breakpoint, do_tbreak);
shell_cmd(next_instr, single step over call instructions, do_next_instruction);
shell_cmd(break_list, list all breakpoints, do_break_list);
shell_cmd(break_del, delete a breakpoint, do_break_del);
