#include "builtins.h"
#include "config.h"
#include "shell.h"
#include "common.h"
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>

static int
accept_builtin(WORD_LIST *list)
{
    if (list == NULL) {
        builtin_usage();
        return EX_USAGE;
    }

    if (no_options(list)) {
        return EX_USAGE;
    }

    const char *varname = list->word->word;

    list = list->next;
    
    /* Default to STDIN */
    int socket = 0;
    
    /* Pick up socket file descriptor from argument if specified. */
    if (list != NULL) {
        if (sscanf(list->word->word, "%d", &socket) < 1) {
            builtin_error("invalid socket fd: %s", list->word->word);
            return EXECUTION_FAILURE;
        }
        list = list->next;
    }
    
    if (list != NULL) {
        builtin_usage();
        return EX_USAGE;
    }

    int accfd = accept(socket, NULL, NULL);
    if (accfd == -1) {
        builtin_error("%s", strerror(errno));
        return EXECUTION_FAILURE;
    }

    /* Bind file description number to varname. */
    char buffer[32];
    int ret = snprintf(buffer, sizeof(buffer), "%d", accfd);
    if (ret < 0 || ((unsigned int)ret) >= sizeof(buffer)) {
        builtin_error("snprintf: %s", strerror(errno));
        return EXECUTION_FAILURE;
    }
    bind_variable(varname, buffer, 0);

    return EXECUTION_SUCCESS;
}

static char *const accept_doc[] = {
    "accept a connection on a socket.",
    "Calls accept(2) on the given socket. The resulting file descriptor",
    "is written to the variable denoted by varname.", 
    "if socket is not given, assumes 0 (STDIN).", NULL
};

struct builtin accept_struct = {
    .name = "accept",
    .function = accept_builtin,
    .flags = BUILTIN_ENABLED,
    .long_doc = accept_doc,
    .short_doc = "accept varname [socket]",
    .handle = NULL
};
