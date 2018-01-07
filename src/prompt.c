#include <err.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include "cmd.h"
#include "dbg.h"

struct ctx g_ctx;

static void init_ctx(void)
{
    g_ctx.binary = NULL;
    g_ctx.child_pid = 0;
    g_ctx.bp_list = bplist_init();
}

static void prompt(void)
{
    for (;;) {
        char *cmd = readline("dbg> ");
        if (!cmd) {
            putchar('\n');
            free(g_ctx.bp_list);
            exit(0);
        }
        int res = exec_cmd(cmd);
        free(cmd);
        if (res == -1)
            break;
    }
}

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        warnx("Too many arguments given\n");
        free(g_ctx.bp_list);
        return 1;
    }
    init_ctx();
    if (argc == 2) {
        pid_t pid = fork();
        if (pid == -1) {
            free(g_ctx.bp_list);
            return 1;
        }
        else if (pid == 0) {
            if (ptrace(PTRACE_TRACEME, 0, 0, 0) == -1) {
                free(g_ctx.bp_list);
                return 1;
            }
            execvp(argv[1], argv + 1);
        }
        else {
            g_ctx.binary = argv[1];
            g_ctx.child_pid = pid;
            prompt();
            free(g_ctx.bp_list);
            //kill(g_ctx.child_pid, SIGTERM);
        }
    }
    return 0;
}
