#include "ksimple_pstree.h"

static void udp_reply(int pid,int seq,void *payload)
{
    struct sk_buff *skb;
    struct nlmsghdr *nlh;
    int size=strlen(payload)+1;
    int len = NLMSG_SPACE(size);
    void *data;
    int ret;

    skb = alloc_skb(len, GFP_ATOMIC);
    if (!skb)
        return;
    nlh= nlmsg_put(skb, pid, seq, 0, size, 0);
    nlh->nlmsg_flags = 0;
    data=NLMSG_DATA(nlh);
    memcpy(data, payload, size);
    NETLINK_CB(skb).portid = 0; /* from kernel */
    NETLINK_CB(skb).dst_group = 0; /* unicast */
    ret=netlink_unicast(netlink_sock, skb, pid, MSG_DONTWAIT);
    if (ret <0) {
        printk("send failed\n");
        return;
    }
    return;
}

static void udp_receive(struct sk_buff *skb)
{
    u_int pid, seq, sid;
    void *data;
    struct nlmsghdr *nlh;

    nlh = (struct nlmsghdr *)skb->data;
    pid = NETLINK_CREDS(skb)->pid;
    sid = NETLINK_CB(skb).nsid;
    seq = nlh->nlmsg_seq;
    data = NLMSG_DATA(nlh);
    // pid code is herei
    int result;
    char pidpid[4096];
    int tabtime;
    struct pid * kpid;
    struct task_struct *task;
    //check -c -s -p
    sprintf(pidpid, "");
    if (((char*)data)[0] != '-') {
        result = (int)simple_strtol((char*)(data), NULL, 10);
        tabtime = 1;
        if(find_get_pid(result) != NULL) {
            kpid = find_get_pid(result);
            task = pid_task(kpid, PIDTYPE_PID);
            sprintf(pidpid, "%s(%d)\n", task->comm, task->pid);
            find_children(task, pidpid, tabtime);
        } else	sprintf(pidpid, "");
    } else if(((char*)data)[0] == '-' && ((char*)data)[1] == 'c') {
        result = (int)simple_strtol((char*)(data + 2), NULL, 10);
        tabtime = 1;
        if(find_get_pid(result) != NULL) {
            kpid = find_get_pid(result);
            task = pid_task(kpid, PIDTYPE_PID);
            sprintf(pidpid, "%s(%d)\n", task->comm, task->pid);
            find_children(task, pidpid, tabtime);
        } else    sprintf(pidpid, "");
    } else if(((char*)data)[0] == '-' && ((char*)data)[1] == 's') {
        if(strcmp((char*)data, "-s") != 0) {
            result = (int)simple_strtol((char*)(data + 2), NULL, 10);
            if(find_get_pid(result) != NULL) {
                kpid = find_get_pid(result);
                task = pid_task(kpid, PIDTYPE_PID);
                find_sibling(task, pidpid);
            } else    sprintf(pidpid, "");
        } else {
            int simple_pstree_pid;
            kpid = find_get_pid(1);
            task = pid_task(kpid, PIDTYPE_PID);
            find_simple_pstree(task, &simple_pstree_pid);
            kpid = find_get_pid(simple_pstree_pid);
            task = pid_task(kpid, PIDTYPE_PID);
            find_sibling(task, pidpid);

        }
    } else if(((char*)data)[0] == '-' && ((char*)data)[1] == 'p') {
        if(strcmp((char*)data, "-p") != 0) {
            result = (int)simple_strtol((char*)(data + 2), NULL, 10);
            tabtime = 0;
            if(find_get_pid(result) != NULL) {
                kpid = find_get_pid(result);
                task = pid_task(kpid, PIDTYPE_PID);
                find_parent(task, pidpid, tabtime);
            } else    sprintf(pidpid, "");
        } else {
            tabtime = 0;
            int simple_pstree_pid;
            kpid = find_get_pid(1);
            task = pid_task(kpid, PIDTYPE_PID);
            find_simple_pstree(task, &simple_pstree_pid);
            kpid = find_get_pid(simple_pstree_pid);
            task = pid_task(kpid, PIDTYPE_PID);
            find_parent(task, pidpid, tabtime);

        }
    } else    sprintf(pidpid, "");
    //
    udp_reply(pid,seq,pidpid);
    return ;
}

static void find_children(struct task_struct *task, char pidpid[], int tabtime)
{
    struct list_head *p = NULL;
    char name[128];
    list_for_each(p, &task->children) {
        int i;
        for(i=0; i<tabtime; ++i)	strcat(pidpid, "    ");
        struct task_struct *temp = list_entry(p, struct task_struct, sibling);
        sprintf(name, "%s(%d)\n", temp->comm, temp->pid);
        strcat(pidpid, name);
        find_children(temp, pidpid, tabtime+1);
    }
}
static void find_sibling(struct task_struct *task, char pidpid[])
{
    struct list_head *p = NULL;
    char name[128];
    list_for_each(p, &task->sibling) {
        struct task_struct *temp = list_entry(p, struct task_struct, sibling);
        if(temp->pid != 0) {
            sprintf(name, "%s(%d)\n", temp->comm, temp->pid);
            strcat(pidpid, name);
        }
    }
}
static void find_parent(struct task_struct *task, char pidpid[], int tabtime)
{
    int i, j;
    int count = 0;
    int pidnumber[100];
    pidnumber[0] = task->pid;
    while((task-> parent) != NULL && (task -> pid) != 1) {
        ++count;
        task = task -> parent;
        pidnumber[count] = task->pid;
        if(task->pid == 1)      break;
    }

    for(j = count; j >= 0; --j) {
        char name[128];
        struct pid * kpid;
        struct task_struct *temp;

        kpid = find_get_pid(pidnumber[j]);
        temp = pid_task(kpid, PIDTYPE_PID);
        for(i=0; i<tabtime; ++i)        strcat(pidpid, "    ");
        ++tabtime;
        sprintf(name, "%s(%d)\n", temp->comm, temp->pid);
        strcat(pidpid, name);

    }

}
static void find_simple_pstree(struct task_struct *task, int *simple_pstree_pid)
{
    struct list_head *p = NULL;
    list_for_each(p, &task->children) {
        struct task_struct *temp = list_entry(p, struct task_struct, sibling);
        if(strcmp((char*)(temp->comm), "simple-pstree") == 0)	*simple_pstree_pid = temp->pid;
        find_simple_pstree(temp, simple_pstree_pid);
    }
}



static int __init kudp_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = udp_receive,
    };
    netlink_sock = netlink_kernel_create(&init_net, NETLINK_USERSOCK, &cfg);
    return 0;
}

static void __exit kudp_exit(void)
{
    sock_release(netlink_sock->sk_socket);
}
module_init(kudp_init);
module_exit(kudp_exit);
