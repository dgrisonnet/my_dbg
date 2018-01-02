#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <unistd.h>

static void prompt(void)
{
    for (;;)
    {
        char *cmd = readline("dbg> ");
        if (!cmd)
        {
            putchar('\n');
            exit(0);
        }
        //exec_cmd(cmd);
    }
}

int main(int argc, char *argv[])
{
    pid_t pid = fork();
    if (pid == 0)
    {
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        if (execvp(argv[1], argv + 1) == -1);
        prompt();
    }
    waitpid(pid, NULL, 0);
    return 0;
}
