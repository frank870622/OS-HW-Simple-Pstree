#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <linux/skbuff.h>
#include <linux/string.h>
#include <linux/pid.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");

static struct sock *netlink_sock;
static void find_children(struct task_struct*, char[], int);
static void find_sibling(struct task_struct*, char[]);
static void find_parent(struct task_struct*, char[], int);
static void find_simple_pstree(struct task_struct *, int *);

