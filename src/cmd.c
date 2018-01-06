#include <stdio.h>
#include <string.h>

#include "cmd.h"

static int do_continue(void *arg)
{
    (void)arg;
    return 1;
}

static int do_quit(void *arg)
{
    (void)arg;
    return 1;
}

static int do_help(void *arg)
{
    (void)arg;
    for (size_t i = 0; i < (size_t)(__stop_cmds - __start_cmds); ++i) {
        struct cmd *tmp = __start_cmds + i;
        printf("%s: %s\n", tmp->cmd, tmp->desc);
    }
    return 1;
}

int exec_cmd(char *cmd)
{
    for (size_t i = 0; i < (size_t)(__stop_cmds - __start_cmds); ++i) {
        struct cmd *tmp = __start_cmds + i;
        if (!strcmp(cmd, tmp->cmd)) {
            (*tmp->fn)(NULL);
        }
    }
    return 1;
}

shell_cmd(info_regs, display_registers, do_info_regs);
shell_cmd(continue, continue program being debugged, do_continue);
shell_cmd(quit, exit, do_quit);
shell_cmd(help, display this help message, do_help);
shell_cmd(info_memory, display_memory, do_info_memory);
