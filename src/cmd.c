#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <capstone/capstone.h>

#include "cmd.h"
#include "dbg.h"

static void examine_assembly(char *data, size_t size)
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
    count = cs_disasm(handle, code, size, 0x1000, 0, &insn);
    if (count > 0) {
        size_t j;
        for (j = 0; j < count; j++) {
            printf("0x%"PRIx64":\t%s\t\t%s\n", insn[j].address,
                   insn[j].mnemonic, insn[j].op_str);
        }
        cs_free(insn, count);
    }
    free(code);
}

static void examine_numbers(char *data, size_t size, size_t type)
{
    int pretty_print = 16;
    for (size_t i = 0; i < size; i += type, pretty_print += type) {
        if (pretty_print == 16) {
            printf("0x%x: ", *(unsigned *)(data + i));
            pretty_print = 0;
        }
        int tmp = 0;
        memcpy(data + i, &tmp, size - 1 - i < type ? size - 1 - i : type);
        if (type == 2)
            printf("0x%02x ", *(unsigned short *)(data + i));
        else
            printf("%d ", *(int *)(data + i));
    }
}

static int examine_print(char *data, char format, size_t size)
{
    switch (format) {
    case 's':
        printf("%s\n", data);
        break;
    case 'd':
        examine_numbers(data, size, 4);
        break;
    case 'x':
        examine_numbers(data, size, 2);
        break;
    case 'i':
        examine_assembly(data, size);
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
                          (void *)(addr + i));
        if (tmp == -1) {
            free(data);
            return 1;
        }
        memcpy(data + i, &tmp, size - 1 - i < 4 ? size - 1 - i : 4);
    }
    int res = examine_print(data, format, size);
    free(data);
    return res;
}

static int do_single_step(void *args)
{
    (void)args;
    if (ptrace(PTRACE_SINGLESTEP, g_ctx.child_pid, 0, 0) == -1)
        return 0;
    int status;
    siginfo_t siginfo;
    wait(&status);
    if (WIFSTOPPED(status)) {
        printf("%s\n", strsignal(WSTOPSIG(status)));
        if (ptrace(PTRACE_GETSIGINFO, g_ctx.child_pid, 0, &siginfo) == -1)
            return 0;
        if (siginfo.si_signo != SIGTRAP)
            return 1;
    }
    else {
        printf("Process terminated\n");
        return 1;
    }
    return 0;
}

static int do_continue(void *args)
{
    (void)args;
    struct user_regs_struct regs;
    if (ptrace(PTRACE_CONT, g_ctx.child_pid, 0, 0) == -1)
        return 0;
    int status;
    wait(&status);
    if (WIFSTOPPED(status))
        printf("%s\n", strsignal(WSTOPSIG(status)));
    else {
        printf("Process terminated\n");
        return 1;
    }
    if (ptrace(PTRACE_GETREGS, g_ctx.child_pid, 0, &regs) == -1)
        return 0;
    void *data = (void *)((get_breakpoint(regs.rip))->content);
    if (ptrace(PTRACE_POKETEXT, g_ctx.child_pid, (void *)regs.rip, data) == -1)
        return 0;
    --regs.rip;
    if (ptrace(PTRACE_SETREGS, g_ctx.child_pid, 0, &regs) == -1)
        return 0;
    return 1;
}

static int do_quit(void *args)
{
    (void)args;
    return -1;
}

static int do_help(void *args)
{
    (void)args;
    for (size_t i = 0; i < (size_t)(__stop_cmds - __start_cmds); ++i) {
        struct cmd *tmp = __start_cmds + i;
        printf("%s: %s\n", tmp->cmd, tmp->desc);
    }
    return 1;
}

static void remove_spaces(char *str)
{
    size_t pos = 0;
    for (size_t i = 0; str[i] != '\0'; ++i)
        if (str[i] != ' ')
            str[pos++] = str[i];
    str[pos] = '\0';
}

int exec_cmd(char *cmd)
{
    int res = 1;
    for (size_t i = 0; i < (size_t)(__stop_cmds - __start_cmds); ++i) {
        struct cmd *tmp = __start_cmds + i;
        if (!strncmp(cmd, tmp->cmd, strlen(tmp->cmd))) {
            char *args = cmd + strlen(tmp->cmd) + 1;
            if (strcmp("examine", tmp->cmd))
                remove_spaces(args);
            res = (*tmp->fn)(args);
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
