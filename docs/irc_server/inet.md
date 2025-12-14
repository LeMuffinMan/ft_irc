# inet #

```C
#include <arpa/inet.h>
```

## inet_addr ##

```C
in_addr_t inet_addr(const char *cp);
```

## inet_ntoa ##

```C
char *inet_ntoa(struct in_addr in);
```

## inet_ntop ##

```C
const char *inet_ntop(socklen_t size; int af, const void *restrict src,
	char dst[restrict size], socklen_t size);
```
