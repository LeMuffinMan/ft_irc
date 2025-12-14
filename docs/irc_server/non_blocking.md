# Non Blocking waiting #

> Checking the value of `errno` to adjust the server behaviour is strictly forbidden after performing a read or write operation.
> > So it's acceptable if used for initialisation of the polling functions ?

`select` is the first created, `poll` is a replacement but created in 1983.
`epoll` (linux) and `kqueue` (macOS) are scalable I/O events that are more efficient when used with a lot of events. `kevent` from kqueue is said to be faster as it combines two syscall as `epoll_ctl` and `epoll_wait` makes.

## select ##

```C
#include <sys/select.h>

int select(int nfds, fd_set *restrict readfds, fd_set *restrict writefds,
    fd_set *restrict errorfds, struct timeval *restrict timeout);
```

## poll ##

```C
#include <poll.h>
 struct pollfd {
    int   fd;         /* file descriptor */
    short events;     /* requested events */
   short revents;    /* returned events */
};
int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```

## epoll ##

### epoll_create ###

```C
#include <sys/epoll.h>
```

```C
/**
 * @brief Create a new epoll instance
 * 
 * @param[in] size number of expected fd, but anything higher than 0 is OK
 * as it's dynamically changed anyways
 * @return a valid fd, -1 if error and errno set
 */
int epoll_create(int size);
```

ERRORS :

* EINVAL size is not positive.
* EMFILE The  per-user  limit  on the number of epoll instances imposed by /proc/sys/fs/epoll/max_user_instances was encountered.  See epoll(7) for further details.
* EMFILE The per-process limit on the number of open file descriptors has been reached.
* ENFILE The system-wide limit on the total number of open files has been reached.
* ENOMEM There was insufficient memory to create the kernel object.

### epoll_ctl ###

```C
/**
 * @brief Add, modify or remove entrieds in interest list
 * 
 * @param[in] epfd file descriptor of epoll instance
 * @param[in] op type of operation
 * @param[in] fd fd we want to add/remove
 * @param[in, out] event event to modify
 * @return 0 OK, -1 if error and errno set
 */
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
```

#### Flags for op ####

* `EPOLL_CTL_ADD`  : register a fd to the interest list
* `EPOLL_CTL_DEL` : delete/deregister fd from interest list (not closing it)
* `EPOLL_CTL_MOD` : modify the event fd
  > if used, you need to set `event` as a pointer to struct `epoll_event`

* `EPOLLIN` : makes epoll track if fd is ready for READ
* `EPOLLOUT` : makes epoll track if fd is ready for WRITE
* `EPOLLET` : makes it 'edge-triggered', you need to keep reading until the end because the next epoll won't remind you that there is still data to write/read.

Epoll usage for a web server socket:

* The listening socket file descriptor (FD) itself is a single FD. It does not contain multiple FDs. Instead, when you accept a new connection on this listening socket, accept() returns a new FD for that connection. So you end up with multiple FDs: one for listening new connections, and one for each client connected.
* epoll monitors multiple FDs simultaneously by registering each FD separately with epoll_ctl. You add the listening socket FD initially, and then for each accepted connection, you add its new FD to the epoll set. epoll_wait then reports readiness events for any of these FDs.
* The array you pass to epoll_wait (events[MAX_EVENTS]) is a buffer you allocate to hold the event notifications from epoll_wait. If more events happen than fit in your array, subsequent epoll_wait calls will provide the rest.
* The "events to keep in memory" means this array you maintain on the user-space side to receive event notifications from the kernel. You should allocate it with a size that balances memory usage and concurrency needs.
* Each epoll_event contains a field data.fd which is the FD that generated the event, so your code can distinguish events from different connections.

To summarize: your listening socket FD is single, but accept() creates many FDs for client connections. You add all these FDs to epoll.

### epoll_wait ###

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

```C
/**
 * @brief Waits for events on the epoll instance 
 * 
 * @param[in] epfd file descriptor of epoll instance
 * @param[in, out] event
 * @param[in] maxevents maximum number of events returned (>0)
 * @param[in] timeout number of milliseconds before epoll_wait() unblocks itself, -1 is infinite
 * @return number of fd ready for I/O, 0 if timeout, -1 if error and errno set
 */
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```

* `events` : need to be a contiguous block of events, a vector can grow and shrink and is contiguous

One implementation of events instead of a tab of events (`event[SIZE]`), nfds is the current number of fds modified :

```C
if (nfds == events.size())
    events.resize(events.size() * 2); // or just a .push_back() to not double the size
else if (nfds < events.size() / 2 && events.size() > MAX_EVENTS_SIZE) //if events is becoming too big, we can shrink it
    events.resize(events.size() / 2);
```

## kqueue / kevent ##

> [!WARNING]
> Only on BSD and macOS, so we can't use it. We could use them if we want to make port the program.

```C
#include <sys/event.h>
```

### kqueue ###

```C
/**
 * @brief Creates a new kernel event queue and returns a descriptor
 * 
 * @return descriptor, -1 if error and errno set 
 */
int kqueue(void);
```

### kevent ###

```C
struct kevent {
    uintptr_t ident;   /* identifier for this event */
    short filter;      /* filter for event */
    u_short flags;     /* action flags for kqueue */
    u_int fflags;      /* filter flag value */
    int64_t ata;       /* filter data value */
    void *udata;       /* opaque user data identifier */
    uint64_t ext[4];   /* extensions */
};

struct timespec {
    time_t tv_sec;      // nombre de secondes
    long tv_nsec;       // nombre de nanosecondes
};

/**
 * @brief Register events with the queue and 
 * 
 * @param[in] kq descriptor of kqueue instance
 * @param[in, out] changelist list of events that we WANT to change
 * @param[in] nchanges size of changelist
 * @param[in, out] eventlist list of events that are ready
 * @param[in] nevents size of eventlist
 * @param[in] timeout timespect, if NULL, waits indefinitely
 * @return number of events placed in eventlist, 0 if timeout, -1 if error and errno set 
 */
int kevent(int kq, const struct    kevent *changelist,     int nchanges,
struct kevent *eventlist, int nevents, const struct    timespec *timeout);
```
