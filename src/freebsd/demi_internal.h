#include <sys/param.h>
#include <sys/socket.h>

#define DEMI_CLOEXEC SOCK_CLOEXEC
#define DEMI_NONBLOCK SOCK_NONBLOCK

#define DEMI_DEVNAME_MAX (SPECNAMELEN + 1)
