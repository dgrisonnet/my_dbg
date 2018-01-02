#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cmd.h>

static void prompt(void)
{
    for (;;) {
        char *cmd = readline("dbg> ");
        if (!cmd) {
            putchar('\n');
            exit(0);
        }
        exec_cmd(cmd);
    }
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    prompt();
    return 0;
}
