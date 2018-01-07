#ifndef BREAKPOINT_H
# define BREAKPOINT_H

#include <stddef.h>
#include <stdint.h>

# define CONTAINER_OF(Typename, Fieldname, Ptr)                               \
    ((Typename *)((char *)Ptr - offsetof(Typename, Fieldname)))

/**
 * struct bplist_node - Intrusive breakpoint list node.
 * @next: Next element of the intrusive list.
 *
 * Structure used to store the node of the intrusive list.
 */

struct bplist_node
{
    struct bplist_node *next;
};

/**
 * enum bp_type - Enumeration of breakpoint types.
 *
 * This enumeration allow us to differentiate the differement types of
 * breakpoints while running the program.
 */

enum bp_type
{
    BREAK = 0,
    TBREAK,
};

/**
 * struct breakpoint - Intrusive breakpoint list element.
 * @addr: Address of the breakpoint.
 * @content: Data which was previously at this address.
 * @id: Id of the breakpoint.
 * @type: Type of the breakpoint.
 * @node: Node stored inside of the intrusive list.
 *
 * Structure used to store our breakpoints information.
 */

struct breakpoint
{
    long addr;
    long content;
    long id;
    enum bp_type type;
    struct bplist_node node;
};

/**
 * struct bp_list - Intrusive list containing the breakpoints.
 * @size: Number of breakpoints store in the list.
 * @id: Id of the next breakpoint.
 * @head: Head of the list.
 *
 * Intrusive list containing all the breakpoints set by the user.
 */

struct bp_list
{
    size_t size;
    long id;
    struct bplist_node *head;
};

/**
 * bplist_init() - Init the breakpoint list.
 *
 * Return: a bp_list if everything gone well otherwise NULL.
 */

struct bp_list *bplist_init(void);

/**
 * bplist_add() - Add a node to the list.
 * @arg1: Node we want to add.
 *
 * Add a new node part of a breakpoint structure to the list.
 */

void bplist_add(struct bplist_node *node);

/**
 * bplist_remove() - Remove a breakpoint from the list.
 * @arg1: Id of the breakpoint we want to remove.
 *
 * Remove the breakpoint which as an id corresponding to the one given in
 * argument. If none were find it prints an error.
 */

void bplist_remove(long id);

/**
 * bplist_destroy() - Free all breakpoints.
 */

void bplist_destroy(void);

/**
 * get_breakpoint() - Get a breakpoint from the list.
 * @arg1: Address of the breakpoint we want to get.
 *
 * Get the first breakpoint which address correspond to the one given in
 * argument from the list.
 *
 * Return: the breakpoint if it was found else NULL;
 */

struct breakpoint *get_breakpoint(long addr);

/**
 * do_break_list() - Execute break_list command.
 * @arg1: Unused argument.
 *
 * Execute the break_list command inside of the debugger which is adding
 * printing all the breakpoints.
 *
 * Return: 1.
 */

int do_break_list(void *args);

/**
 * do_tbreak() - Execute tbreak command.
 * @arg1: Argument of breakpoint command it must be an hexadecimal address.
 *
 * Execute the tbreak command inside of the debugger which is adding a
 * temporary breakpoint.
 *
 * Return: 1 if everything went well otherwise 0.
 */

int do_tbreak(void *args);

/**
 * do_break() - Execute break command.
 * @arg1: Argument of breakpoint command it must be an hexadecimal address.
 *
 * Execute the break command inside of the debugger.
 *
 * Return: 1 if everything went well otherwise 0.
 */

int do_break(void *args);

#endif /* !BREAKPOINT_H */
