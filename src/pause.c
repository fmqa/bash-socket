#include "builtins.h"
#include "config.h"
#include "shell.h"
#include "common.h"
#include <unistd.h>

static int
pause_builtin(WORD_LIST *list)
{
    if (no_options(list)) {
        return EX_USAGE;
    }
    /* Don't accept any arguments. */
    if (list != NULL) {
        builtin_usage();
        return EX_USAGE;
    }

    (void)pause();

    return EXECUTION_SUCCESS;
}

static char *const pause_doc[] = {
    "Suspends the shell until a signal is delivered.", NULL
};

struct builtin pause_struct = {
    .name = "pause",
    .function = pause_builtin,
    .flags = BUILTIN_ENABLED,
    .long_doc = pause_doc,
    .short_doc = "pause",
    .handle = NULL
};
