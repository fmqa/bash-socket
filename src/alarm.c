#include "builtins.h"
#include "config.h"
#include "shell.h"
#include "common.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

static int
alarm_builtin(WORD_LIST *list)
{
    if (list == NULL) {
        builtin_usage();
        return EX_USAGE;
    }

    if (no_options(list)) {
        return EX_USAGE;
    }

    unsigned int seconds;

    if (sscanf(list->word->word, "%u", &seconds) != 1) {
        builtin_error("invalid argument: %s", list->word->word);
        return EXECUTION_FAILURE;
    }

    list = list->next;

    unsigned int remaining = alarm(seconds);

    if (list != NULL) {
        char buffer[16];
        int ret = snprintf(buffer, sizeof(buffer), "%u", remaining);
        if (ret < 0 || ((unsigned int)ret) >= sizeof(buffer)) {
            builtin_warning("snprintf: %s", strerror(errno));
        } else {
            bind_variable(list->word->word, buffer, 0);
        }
    }

    return EXECUTION_SUCCESS;
}

static char *const alarm_doc[] = {
    "Arranges for a SIGALRM to be delivered to the shell in seconds.",
    "If seconds is zero, any pending alarm is cancelled.",
    "If varname is given, it is set to the number of seconds remaining until any",
    "previously scheduled alarm was to be delivered, or zero if there was",
    "no previously scheduled alarm.", NULL
};

struct builtin alarm_struct = {
    .name = "alarm",
    .function = alarm_builtin,
    .flags = BUILTIN_ENABLED,
    .long_doc = alarm_doc,
    .short_doc = "alarm seconds [varname]",
    .handle = NULL
};
