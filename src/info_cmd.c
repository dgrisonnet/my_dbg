#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/user.h>
#include <sys/ptrace.h>

#include "cmd.h"
#include "dbg.h"

int do_info_memory(void *UNUSED(args))
{
    pid_t pid = getpid();
    char *path = calloc(MAPPING_PATH, 1);
    if (!path)
        return 0;
    sprintf(path, "/proc/%u/maps", pid);
    FILE *f = fopen(path, "r");
    if (!f) {
        printf("Could not open memory mapping\n");
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

int do_info_regs(void *UNUSED(args))
{
    struct user_regs_struct regs;
    if (ptrace(PTRACE_GETREGS, g_ctx.child_pid, NULL, &regs) == -1) {
        printf("ptrace fail\n");
        return 0;
    }
    printf("rip: 0x%llx\n", regs.rip);
    printf("rsp: 0x%llx\n", regs.rsp);
    printf("rbp: 0x%llx\n", regs.rbp);
    printf("eflags: 0x%llx\n", regs.eflags);
    printf("orig_rax: 0x%llx\n", regs.orig_rax);
    printf("rax: 0x%llx\n", regs.rax);
    printf("rbx: 0x%llx\n", regs.rbx);
    printf("rcx: 0x%llx\n", regs.rcx);
    printf("rdx: 0x%llx\n", regs.rdx);
    printf("rdi: 0x%llx\n", regs.rdi);
    printf("rsi: 0x%llx\n", regs.rsi);
    printf("r8: 0x%llx\n", regs.r8);
    printf("r9: 0x%llx\n", regs.r9);
    printf("r10: 0x%llx\n", regs.r10);
    printf("r11: 0x%llx\n", regs.r11);
    printf("r12: 0x%llx\n", regs.r12);
    printf("r13: 0x%llx\n", regs.r13);
    printf("r14: 0x%llx\n", regs.r14);
    printf("r15: 0x%llx\n", regs.r15);
    printf("cs: 0x%llx\n", regs.cs);
    printf("ds: 0x%llx\n", regs.ds);
    printf("es: 0x%llx\n", regs.es);
    printf("fs: 0x%llx\n", regs.fs);
    printf("gs: 0x%llx\n", regs.gs);
    printf("ss: 0x%llx\n", regs.ss);
    printf("fs_base: 0x%llx\n", regs.fs_base);
    printf("gs_base: 0x%llx\n", regs.gs_base);
    return 1;
}
