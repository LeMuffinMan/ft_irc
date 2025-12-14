# Socket #

All functions used for socket programming.

## getprotobyname ##

```C
#include <netdb.h>
struct protoent {
    char  *p_name;       /* official protocol name */
    char **p_aliases;    /* alias list */
    int    p_proto;      /* protocol number */
}
struct protoent *getprotobyname(const char *name);
```

## Shared within socket functions ##

```C
#include <sys/socket.h>

struct sockaddr {
    sa_family_t     sa_family;      /* Address family */
    char            sa_data[];      /* Socket address */
};
typedef /* ... */ sa_family_t; /* Describes a socket's protocol family. This is an unsigned integer type. */
typedef /* ... */ socklen_t; /* Describes the length of a socket address.  This is an integer type of at least 32 bits. */
```

## socket ##

```C
int socket(int domain, int type, int protocol);
```

## accept ##

```C
int accept(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
```

## listen ##

```C
int listen(int socket, int backlog);
```

## send ##

```C
ssize_t send(int socket, const void *buffer, size_t length, int flags);
```

## recv ##

```C
ssize_t recv(int socket, void *buffer, size_t length, int flags);
```

## bind ##

```C
int bind(int socket, const struct sockaddr *address, socklen_t address_len);
```

## connect ##

```C
int connect(int socket, const struct sockaddr *address, socklen_t address_len);
```

## setsockopt ##

```C
int setsockopt(int socket, int level, int option_name, const void *option_value, 
   socklen_t option_len);
```

## getsockname ##

```C
int getsockname(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len);
```
