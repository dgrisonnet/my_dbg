#ifndef CMD_H
# define CMD_H

#define ALIGNED_CMD_SIZE 32
#define MAPPING_PATH 17

/** 
 * struct cmd - Debugger commands.
 * @cmd: String of the command.
 * @desc: Short description of the command.
 * @fn: Function pointer to execute the command.
 *
 * Structure used to store all the commands used by the debugger.
 */

struct cmd {
    const char *cmd;
    const char *desc;
    int (*fn)(void *);
}__attribute__((aligned (ALIGNED_CMD_SIZE)));

#define shell_cmd(name, info, func) \
    static struct cmd __cmd_ ## name \
    __attribute__ ((section("cmds"), used)) = \
    { .cmd = #name, .desc = #info, .fn = func }

extern struct cmd __start_cmds[];
extern struct cmd __stop_cmds[];

/**
 * exec_cmd() - Execute commands.
 * @arg1: String of the command passed by the user.
 *
 * Execute all the commands given by the user inside of the prompt.
 *
 * Return: 1 if a command was found and executed well otherwise 0.
 */

int exec_cmd(char *cmd);

int do_info_regs(void *arg);

int do_info_memory(void *arg);

#endif /* !CMD_H */
