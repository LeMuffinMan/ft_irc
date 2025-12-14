# Convertion host to network order #

## Basics ##

```C
#include <arpa/inet.h>
// Or some system might need instead
// #include <netinet/in.h>
```

Toutes les fonctions pour convertir des valeurs entre un hôte et le réseau, pour avoir le même ordre des octets.

* The `htonl()` function converts the unsigned integer hostlong  from  host  byte
       order to network byte order.
* The  `htons()` function converts the unsigned short integer hostshort from host
byte order to network byte order.
* The `ntohl()` function converts the unsigned integer netlong from network  byte
order to host byte order.
* The  `ntohs()`  function converts the unsigned short integer netshort from net‐
work byte order to host byte order.

### Host -> Network ###

```C
uint32_t htonl(uint32_t hostlong);
uint16_t htons(uint16_t hostshort);
```

### Network -> Host ###

```C
uint32_t ntohl(uint32_t netlong);
uint16_t ntohs(uint16_t netshort);
```
