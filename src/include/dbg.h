#ifndef DBG_H
# define DBG_H

#include <sys/types.h>

#include "breakpoint.h"

struct ctx {
    char *binary;
    pid_t child_pid;
    struct bp_list *bp_list;
};

extern struct ctx g_ctx;

#endif /* !DBG_H */
