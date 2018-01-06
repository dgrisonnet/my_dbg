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
        free(cmd);
    }
}

int main(int argc, char *argv[])
{
    (void)argc;
    pid_t pid = fork();
    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        execvp(argv[1], argv + 1);
    }
    waitpid(pid, NULL, 0);
    prompt();
    return 0;
}
