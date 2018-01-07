#ifndef DBG_H
# define DBG_H

#include <sys/types.h>

#include "breakpoint.h"

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

struct ctx {
    char *binary;
    pid_t child_pid;
    struct bp_list *bp_list;
};

extern struct ctx g_ctx;

#endif /* !DBG_H */
