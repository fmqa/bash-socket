# bash-extensions

Loadable BASH extensions for socket programming.

## Loading the extension

`$ enable -f /usr/lib/x86_64-linux-gnu/libbashext-socket.so accept socket alarm pause`

## Provided functions

The extension provides the following functions

### accept(2)

```
$ help accept
accept: accept varname [socket]
    accept a connection on a socket.

    Calls accept(2) on the given socket. The resulting file descriptor
    is written to the variable denoted by varname.
    if socket is not given, assumes 0 (STDIN).
```

### socket(2)

```
$ help socket
socket: socket varname AF_UNIX|AF_INET|AF_INET6 SOCK_STREAM|SOCK_DGRAM local|peer PATH|ADDRESS [port] [queue]
    creates a socket.

    Provides an interface for creating and using POSIX sockets
    The result socket handle is written to the variable denoted by varname.
```

### alarm(2)

```
$ help alarm
alarm: alarm seconds [varname]
    Arranges for a SIGALRM to be delivered to the shell in seconds.
    If seconds is zero, any pending alarm is cancelled.
    If varname is given, it is set to the number of seconds remaining until any
    previously scheduled alarm was to be delivered, or zero if there was
    previously scheduled alarm.
```

### pause(2)

```
$ help pause
pause: pause
    Suspends the shell until a signal is delivered.
```

## Examples

### Forking AF\_UNIX server

Code (`echo-server.sh`):

```bash
#!/bin/bash
set -eu

enable -f "/usr/lib/$HOSTTYPE-$OSTYPE/libbashext-socket.so" socket accept

socket SOCK AF_UNIX SOCK_STREAM local "$1"

trap "trap - TERM && kill -- -$$ 2>/dev/null" INT TERM HUP QUIT EXIT

while accept FD "${SOCK}" 2>/dev/null
do
    while read -r DATA
    do
        printf '%s\n' "${DATA}"
    done <&${FD} >&${FD} &
    exec {FD}>&-
    wait
done
```

Demo:

```
$ bash socket-server.sh /tmp/sock &
[1] 31281
$ nc -N -U /tmp/sock <<< "Hello World"
Hello World
```

### Simple AF\_UNIX client

Code (`echo-client.sh`)

```bash
#!/bin/bash
set -eu

enable -f "/usr/lib/$HOSTTYPE-$OSTYPE/libbashext-socket.so" socket

socket SOCK AF_UNIX SOCK_STREAM peer "$1"

echo "Hello World!" >&${SOCK}
read -r -u ${SOCK} REPLY
echo "$REPLY"
```

Demo:

```
$ bash echo-server.sh /tmp/sock &
[1] 12991
$ bash echo-client.sh /tmp/sock
Hello World!
```
