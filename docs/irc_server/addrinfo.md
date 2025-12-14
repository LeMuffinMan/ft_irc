# Addrinfo #

Network address and service translation

## Basics ##

```C
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
```

```C
//ai : addrinfo
struct addrinfo {
    int              ai_flags;
    int              ai_family;
    int              ai_socktype;
    int              ai_protocol;
    socklen_t        ai_addrlen;
    struct sockaddr *ai_addr;
    char            *ai_canonname;
    struct addrinfo *ai_next; //pointer to next addrinfo
};
```

### gethostbyname ###

```C
extern int h_errno;
struct hostent *gethostbyname(const char *name);
```

### getaddrinfo ###

```C
int getaddrinfo(const char *node, const char *service,
    const struct addrinfo *hints, struct addrinfo **res);
```

### freeaddrinfo ###

```C
void freeaddrinfo(struct addrinfo *res);
```

### gai_strerror ###

```C
const char *gai_strerror(int errcode);
```

The `gai_strerror()` function translates these error codes (from getaddrinfo) to a human readable string, suitable for  error reporting.
