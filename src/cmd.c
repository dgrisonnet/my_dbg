#include <stdio.h>
#include <string.h>
#include <sys/user.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include "cmd.h"
#include "dbg.h"

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
    else
    {
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
