#include "simple_pstree.h"

int main(int argc, char*argv[])
{
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_pid = getpid();
    sa.nl_groups = 0;
    bind(sock_fd, (struct sockaddr*)&sa, sizeof(sa));

    memset(&da, 0, sizeof(da));
    da.nl_family = AF_NETLINK;
    da.nl_pid = 0;
    da.nl_groups = 0;

    nh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));

    nh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nh->nlmsg_pid = getpid();
    nh->nlmsg_flags = 0;

    if(argc <= 1)	strcpy(NLMSG_DATA(nh), "1");
    else {

        if(strcmp(argv[1], "-c") == 0)	strcpy(NLMSG_DATA(nh), "-c1");
        else if(strcmp(argv[1], "-s") == 0)      strcpy(NLMSG_DATA(nh), "-s");
        else if(strcmp(argv[1], "-p") == 0)      strcpy(NLMSG_DATA(nh), "-p");
        else	strcpy(NLMSG_DATA(nh), argv[1]);
    }

    iov.iov_base = (void *)nh;
    iov.iov_len = nh->nlmsg_len;

    msg.msg_name = (void *)&da;
    msg.msg_namelen = sizeof(da);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    sendmsg(sock_fd, &msg, 0);

    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    printf("%s\n",NLMSG_DATA(msg.msg_iov->iov_base));

    /* Close Netlink Socket */
    close(sock_fd);
    return 0;
}
