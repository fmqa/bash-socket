#include "builtins.h"
#include "config.h"
#include "shell.h"
#include "common.h"
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

static int
socket_builtin(WORD_LIST *list)
{
    if (list == NULL) {
        builtin_usage();
        return EX_USAGE;
    }

    if (no_options(list)) {
        return EX_USAGE;
    }

    /* Result variable name */
    const char *varname = list->word->word;

    list = list->next;
    if (list == NULL) {
        builtin_usage();
        return EX_USAGE;
    }

    int af;

    /* Parse socket domain: unix|inet|inet6 */
    if (strcmp("AF_INET", list->word->word) == 0) {
        af = AF_INET;
    } else if (strcmp("AF_INET6", list->word->word) == 0) {
        af = AF_INET6;
    } else if (strcmp("AF_UNIX", list->word->word) == 0) {
        af = AF_UNIX;
    } else {
        builtin_usage();
        return EX_USAGE;
    }

    list = list->next;
    if (list == NULL) {
        builtin_usage();
        return EX_USAGE;
    }

    int socktype;

    /* Parse socket type keyword. */
    if (strcmp("SOCK_STREAM", list->word->word) == 0) {
        socktype = SOCK_STREAM;
    } else if (strcmp("SOCK_DGRAM", list->word->word) == 0) {
        socktype = SOCK_DGRAM;
    } else {
        builtin_usage();
        return EX_USAGE;
    }

    list = list->next;
    if (list == NULL) {
        builtin_usage();
        return EX_USAGE;
    }

    /* 0 = bind + listen, 1 = connect */
    int op;
    if (strcmp("local", list->word->word) == 0) {
        op = 0;
    } else if (strcmp("peer", list->word->word) == 0) {
        op = 1;
    } else {
        builtin_usage();
        return EX_USAGE;
    }

    list = list->next;
    if (list == NULL) {
        builtin_usage();
        return EX_USAGE;
    }

    union {
        struct sockaddr_in in;
        struct sockaddr_in6 in6;
        struct sockaddr_un un;
    } addr;

    socklen_t addrlen;

    switch (af) {
        case AF_UNIX:
            {
                addr.un.sun_family = AF_UNIX;

                /* POSIX standard defines maximum socket name to 108. */
                size_t len = strlen(list->word->word);
                if (len > 107) {
                    builtin_error("invalid socket name: %s", list->word->word);
                    return EXECUTION_FAILURE;
                }
                strncpy(addr.un.sun_path, list->word->word, len + 1);

                addrlen = sizeof(addr.un);
            }
            break;
        case AF_INET:
            {
                addr.in.sin_family = AF_INET;

                if (inet_pton(AF_INET, list->word->word, &addr.in.sin_addr) != 1) {
                    builtin_error("inet_pton: invalid address: %s", list->word->word);
                    return EXECUTION_FAILURE;
                }

                list = list->next;
                if (list == NULL) {
                    builtin_usage();
                    return EX_USAGE;
                }

                uint16_t port;
                if (sscanf(list->word->word, "%"SCNu16, &port) != 1) {
                    builtin_error("invalid port: %s", list->word->word);
                    return EXECUTION_FAILURE;
                }
                addr.in.sin_port = htons(port);

                addrlen = sizeof(addr.in);
            }
            break;
        case AF_INET6:
            {
                addr.in6.sin6_family = AF_INET6;

                if (inet_pton(AF_INET6, list->word->word, &addr.in6.sin6_addr) != 1) {
                    builtin_error("inet_pton: invalid address: %s", list->word->word);
                    return EXECUTION_FAILURE;
                }

                list = list->next;
                if (list == NULL) {
                    builtin_usage();
                    return EX_USAGE;
                }

                uint16_t port;
                if (sscanf(list->word->word, "%"SCNu16, &port) != 1) {
                    builtin_error("invalid port: %s", list->word->word);
                    return EXECUTION_FAILURE;
                }
                addr.in6.sin6_port = htons(port);

                addrlen = sizeof(addr.in6);
            }
            break;
    }

    list = list->next;

    if (op == 0) {
        /* Listen */
        unsigned int queue = 5;
        if (list != NULL) {
            if (sscanf(list->word->word, "%u", &queue) < 1) {
                builtin_error("invalid queue size: %s", list->word->word);
                return EXECUTION_FAILURE;
            }
            list = list->next;
        }
        /* Fail on extraneous arguments. */
        if (list != NULL) {
            builtin_usage();
            return EX_USAGE;
        }

        int sockfd = socket(af, socktype, 0);
        if (sockfd == -1) {
            builtin_error("%s", strerror(errno));
            return EXECUTION_FAILURE;
        }

        if (bind(sockfd, (struct sockaddr *)&addr, addrlen) == -1) {
            close(sockfd);
            builtin_error("bind: %s", strerror(errno));
            return EXECUTION_FAILURE;
        }

        if (listen(sockfd, queue) == -1) {
            close(sockfd);
            builtin_error("listen: %s", strerror(errno));
            return EXECUTION_FAILURE;
        }

        char value[32];
        int ret = snprintf(value, sizeof(value), "%d", sockfd);
        if (ret < 0 || ((unsigned int)ret) >= sizeof(value)) {
            close(sockfd);
            builtin_error("snprintf: %s", strerror(errno));
            return EXECUTION_FAILURE;
        }
        bind_variable(varname, value, 0);

        return EXECUTION_SUCCESS;
    } else {
        /* Fail on extraneous arguments. */
        if (list != NULL) {
            builtin_usage();
            return EX_USAGE;
        }
        /* Connect */
        int sockfd = socket(af, socktype, 0);
        if (sockfd == -1) {
            builtin_error("%s", strerror(errno));
            return EXECUTION_FAILURE;
        }

        if (connect(sockfd, (struct sockaddr *)&addr, addrlen) == -1) {
            builtin_error("connect: %s", strerror(errno));
            return EXECUTION_FAILURE;
        }

        char value[32];
        int ret = snprintf(value, sizeof(value), "%d", sockfd);
        if (ret < 0 || ((unsigned int)ret) >= sizeof(value)) {
            close(sockfd);
            builtin_error("snprintf: %s", strerror(errno));
            return EXECUTION_FAILURE;
        }
        bind_variable(varname, value, 0);

        return EXECUTION_SUCCESS;
    }
}

static char *const socket_doc[] = {
    "creates a socket.",
    "Provides an interface for creating and using POSIX sockets",
    "The result socket handle is written to the variable denoted by varname.", NULL
};

struct builtin socket_struct = {
    .name = "socket",
    .function = socket_builtin,
    .flags = BUILTIN_ENABLED,
    .long_doc = socket_doc,
    .short_doc = "socket varname AF_UNIX|AF_INET|AF_INET6 SOCK_STREAM|SOCK_DGRAM local|peer PATH|ADDRESS [port] [queue]",
    .handle = NULL
};
