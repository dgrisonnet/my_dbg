#define UNW_LOCAL_ONLY

#include <libunwind.h>
#include <stdio.h>
#include <string.h>

#include "cmd.h"

static void info_reg(unw_cursor_t *cursor, unw_regnum_t regnum, char *reg)
{
    unw_word_t data;
    unw_get_reg(cursor, regnum, &data);
    printf("%s: 0x%lx\n", reg, (long)data);
}

static int do_info_regs(void *arg)
{
    (void)arg;
    unw_cursor_t cursor;
    unw_context_t uc;
    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);
    info_reg(&cursor, UNW_X86_64_RIP, "rip");
    info_reg(&cursor, UNW_X86_64_RSP, "rsp");
    info_reg(&cursor, UNW_X86_64_RBP, "rbp");
    info_reg(&cursor, UNW_X86_EFLAGS, "eflags"); // TO FIX
    //info_reg(&cursor, UNW_X86_64_RBP, "orig_rax"); // TO FIX
    info_reg(&cursor, UNW_X86_64_RAX, "rax");
    info_reg(&cursor, UNW_X86_64_RBX, "rbx");
    info_reg(&cursor, UNW_X86_64_RCX, "rcx");
    info_reg(&cursor, UNW_X86_64_RDX, "rdx");
    info_reg(&cursor, UNW_X86_64_RDI, "rdi");
    info_reg(&cursor, UNW_X86_64_RSI, "rsi");
    info_reg(&cursor, UNW_X86_64_R8, "r8");
    info_reg(&cursor, UNW_X86_64_R9, "r9");
    info_reg(&cursor, UNW_X86_64_R10, "r10");
    info_reg(&cursor, UNW_X86_64_R11, "r11");
    info_reg(&cursor, UNW_X86_64_R12, "r12");
    info_reg(&cursor, UNW_X86_64_R13, "r13");
    info_reg(&cursor, UNW_X86_64_R14, "r14");
    info_reg(&cursor, UNW_X86_64_R15, "r15");
    /*info_reg(&cursor, UNW_X86_64_CS, "cs"); // TO FIX
    info_reg(&cursor, UNW_X86_DS, "ds"); // TO FIX
    info_reg(&cursor, UNW_X86_ES, "es"); // TO FIX
    info_reg(&cursor, UNW_X86_FS, "fs"); // TO FIX
    info_reg(&cursor, UNW_X86_GS, "gs"); // TO FIX
    info_reg(&cursor, UNW_X86_SS, "ss"); // TO FIX
    info_reg(&cursor, UNW_X86_64_R15, "fs_base"); // TO FIX
    info_reg(&cursor, UNW_X86_64_R15, "gs_base"); // TO FIX*/
    return 1;
}

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
