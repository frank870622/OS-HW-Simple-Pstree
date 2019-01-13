#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_PAYLOAD 5120  /* maximum payload size*/

struct sockaddr_nl sa, da;
struct nlmsghdr *nh = 0;
int sock_fd;
struct iovec iov;
struct msghdr msg;
