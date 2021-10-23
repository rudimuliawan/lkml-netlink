//
// Created by Rudi Muliawan on 23/10/21.
//

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <linux/string.h>
#include <net/sock.h>
#include "netlink_utils.h"


#define AUTHOR_NAME "RUDI_MULIAWAN"
#define MODULE_DESC "HELLO_WORLD_LKM"


static struct sock *netlink_socket = NULL;
static void netlink_recv_msg_fn(struct sk_buff *skb_in);
static void nlmsg_dump(struct nlmsghdr *nlh);


static struct netlink_kernel_cfg cfg = {
    .input = netlink_recv_msg_fn,
};


static void netlink_recv_msg_fn(struct sk_buff *skb_in)
{
    struct nlmsghdr *nlh_recv, *nlh_reply;
    struct sk_buff *skb_out;
    int user_space_process_port_id;
    int user_space_data_len;
    char *user_space_data;
    char kernel_reply[256];

    printk(KERN_INFO "%s() invoked\n", __FUNCTION__ );

    nlh_recv = (struct nlmsghdr *) (skb_in->data);
    nlmsg_dump(nlh_recv);

    user_space_process_port_id = nlh_recv->nlmsg_pid;
    printk(KERN_INFO "%s(%d) : Port id of the sending userspace process = %u\n",
        __FUNCTION__, __LINE__, user_space_process_port_id
    );

    user_space_data = (char *) nlmsg_data(nlh_recv);
    user_space_data_len = skb_in->len;
    printk(KERN_INFO "%s(%d) : msg recvd from user space = %s, skb_in->len= %d, nlh->nlmsg_len = %d\n",
         __FUNCTION__, __LINE__, user_space_data, user_space_data_len, nlh_recv->nlmsg_len
    );

    if (nlh_recv->nlmsg_flags & NLM_F_ACK) {
        memset(kernel_reply, 0, sizeof(kernel_reply));
        snprintf(
            kernel_reply, sizeof(kernel_reply),
            "Msg from Process %d has been processed by kernel\n", nlh_recv->nlmsg_pid
        );

        skb_out = nlmsg_new(sizeof(kernel_reply), 0);
        nlh_reply = nlmsg_put(skb_out, 0, nlh_recv->nlmsg_seq, NLMSG_DONE, sizeof(kernel_reply), 0);
        strncpy(NLMSG_DATA(nlh_reply), kernel_reply, sizeof(kernel_reply));

        int res;
        res = nlmsg_unicast(netlink_socket, skb_out, user_space_process_port_id);
        if (res < 0) {
            printk(KERN_INFO "Error while sending the data back to the userpsace\n");
            kfree_skb(skb_out);
        }
    }
}


static int __init netlink_greeting_init(void)
{
    printk(KERN_INFO "Hello Kernel\n");
    netlink_socket = netlink_kernel_create(&init_net, NETLINK_TEST_PROTOCOL, &cfg);
    if (!netlink_socket) {
        printk(KERN_INFO "Kernel Netlink Socket for Protocol %d failed\n", NETLINK_TEST_PROTOCOL);
        return -ENOMEM;
    }

    printk(KERN_INFO "Netlink Socket Created Successfully\n");
    return 0;
}


static void __exit netlink_greeting_exit(void)
{
    printk(KERN_INFO "Bye bye\n");
    netlink_kernel_release(netlink_socket);
    netlink_socket = NULL;
}


module_init(netlink_greeting_init);
module_exit(netlink_greeting_exit);


MODULE_AUTHOR(AUTHOR_NAME);
MODULE_DESCRIPTION(MODULE_DESC);
MODULE_LICENSE("GPL");