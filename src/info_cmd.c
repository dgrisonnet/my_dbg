#define UNW_LOCAL_ONLY

#include <libunwind.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"

int do_info_memory(void *arg)
{
    (void)arg;
    pid_t pid = getpid();
    char *path = calloc(MAPPING_PATH, 1);
    if (!path)
        return 0;
    sprintf(path, "/proc/%u/maps", pid);
    FILE *f = fopen(path, "r");
    if (!f) {
        free(path);
        return 0;
    }
    char buffer[64];
    size_t nbread = 0;
    while ((nbread = fread(buffer, 1, 64, f)) != 0)
        fwrite(buffer, 1, nbread, stdout);
    free(path);
    fclose(f);
    return 1;
}

static void info_reg(unw_cursor_t *cursor, unw_regnum_t regnum, char *reg)
{
    unw_word_t data;
    unw_get_reg(cursor, regnum, &data);
    printf("%s: 0x%lx\n", reg, (long)data);
}

int do_info_regs(void *arg)
{
    (void)arg;
    unw_cursor_t cursor;
    unw_context_t uc;
    unw_getcontext(&uc);
    unw_init_local(&cursor, &uc);
    info_reg(&cursor, UNW_X86_64_RIP, "rip");
    info_reg(&cursor, UNW_X86_64_RSP, "rsp");
    info_reg(&cursor, UNW_X86_64_RBP, "rbp");
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
    return 1;
}
