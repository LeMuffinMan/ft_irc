# FT_IRC #

## <a name="introduction-en">📖 Introduction ##

Make your own IRC Server that no one asked for.

## 📋 Table of Contents ##

* [📖 Introduction](#introduction-en)
* [⚙️ Requirements](#requirements-en)
* [📝 Features](#features-en)
* [🚀 How to use](#use-en)

## <a name="requirements-en">⚙️ Requirements ##

The project uses `c++` compiler, with `-std=c++98` as a requirement.

If you want to run the tester, you will need at least a recent version of `cargo` (Rust).

Use only the following `C functions` :

* select, poll, epoll (epoll_create, epoll_ctl,epoll_wait),kqueue (kqueue, kevent)
* getaddrinfo, gethostbyname freeaddrinfo, gai_strerror
* socket, accept, listen, send, recv, bind, connect,setsockopt, getsockname,getprotobyname
* htons, htonl, ntohs, ntohl
* inet_addr, inet_ntoa, inet_ntop
* lseek, close
* signal, sigaction, sigemptyset, sigfillset, sigaddset, sigdelset, sigismember

### How does it work ###

This is the first big project after the `C` projects at 42 so we tried to make it the most Object-Oriented possible as we can now use `C++`.

We used as a reference the [RFC 2812](https://datatracker.ietf.org/doc/html/rfc2812) , and some precisions of [RFC 2811](https://datatracker.ietf.org/doc/html/rfc2811). We implemented only the server, we were not required to implement a client neither a server-to-server communication.

## <a name="features-en">📝 Features ##

Rules for Commands notation:

* Each `<param>` is a string, delimited by a single space (` `).
* Parenthesis such as `(` and `)` are used to show which block can be repeated.
* `*` represent that 0 or more params could be used.
* Square brackets such as `[` and `]` are used to delimit a block that is optionnal.
* Comma (`,`) is used to separate each param inside a same 'block'.
* A param starting with `:` will contain all the next characters in a same param, permit to send a string containing spaces.

### Authentification ###

* PASS `<password>`
* NICK `<nickname>`
* USER `<username>` `<mode>` `<hostname>` `<realname>` : mode and hostname are unused

### Basics ###

* JOIN `#<channel>*(,#<channel>)` `<key1>*(,<key2>)`
* INVITE `<nickname>` `<channel>`
* TOPIC `<channel>` `[ <topic> ]`
* KICK `#<channel>*(,#<channel>)` `#<user>*(,#<user>)` `<comment>`
* MODE (channel only) : `#<channel>` `*(<modes>` `<modeParam>)`
* PRIVMSG `<msgtarget>` `<text to be sent>`
* PART `#<channel>*(,#<channel>)` `<part message>`
* QUIT `<quit message>`

### Bonus ###

* TIME
* PONG

## <a name="use-en">🚀 How to use ##

```sh
./ircserv port password
```

* `port` port number where server will listen and answer incoming IRC connections
* `password` password of the connection, the clients will need it to connect

---
