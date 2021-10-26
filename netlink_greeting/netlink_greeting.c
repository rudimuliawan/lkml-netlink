//
// Created by Rudi Muliawan on 23/10/21.
//

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <net/sock.h>

#include "netlink_utils.h"


static struct sock *nl_sock = NULL;


void netlink_recv_msg_fn(struct sk_buff *skb_in);


static struct netlink_kernel_cfg cfg = {
	.input = netlink_recv_msg_fn
};


void netlink_recv_msg_fn(struct sk_buff *skb_in)
{
	printk(KERN_INFO "%s() invoked", __FUNCTION__ );

	struct nlmsghdr *nlh_recv, *nlh_reply;

	int userspace_port_id;
	char *user_space_data;
	int user_space_data_len;

	struct sk_buff *skb_out;
	char kernel_reply[256];
	int res;

	nlh_recv = (struct nlmsghdr*) skb_in->data;
	nlmsg_dump(nlh_recv);

	userspace_port_id = nlh_recv->nlmsg_pid;
	printk(KERN_INFO "%s(%d) : port id of the sending user space process = %u\n",
		__FUNCTION__, __LINE__, userspace_port_id
	);

	user_space_data = (char *) nlmsg_data(nlh_recv);
	user_space_data_len = skb_in->len;
	printk(KERN_INFO "%s(%d) : msg received from userspace = %s, skb_in->len = %d. nlh->nlmsg_len = %d\n",
		__FUNCTION__, __LINE__, user_space_data, user_space_data_len, nlh_recv->nlmsg_len
	);

	if (nlh_recv->nlmsg_flags == NLM_F_ACK) {
		memset(kernel_reply, 0, sizeof(kernel_reply));
		snprintf(kernel_reply, sizeof(kernel_reply),
			"Message from Process %d has been processed by kernel", nlh_recv->nlmsg_pid
		);

		skb_out = nlmsg_new(sizeof(kernel_reply), 0);
		nlh_reply = nlmsg_put(
			skb_out, 0, nlh_recv->nlmsg_seq, NLMSG_DONE, sizeof(kernel_reply), 0
		);

		strncpy(nlmsg_data(nlh_reply), kernel_reply, sizeof(kernel_reply));
		res = nlmsg_unicast(nl_sock, skb_out, userspace_port_id);
		if (res < 0) {
			printk(KERN_INFO "ERROR while sending the data back to userspace\n");
			kfree_skb(skb_out);
		}
	}
}


static int __init netlink_greetings_init(void)
{
	printk(KERN_INFO "Hello from netlink greetings kernel module\n");

	nl_sock = netlink_kernel_create(&init_net, NETLINK_TEST_PROTOCOL, &cfg);
	if (!nl_sock) {
		printk(KERN_INFO "Kernel Netlink Socket for Netlink Protocol %u failed\n", NETLINK_TEST_PROTOCOL);
		return -ENOMEM;
	}

	return 0;
}


static void __exit netlink_greetings_exit(void)
{
	printk(KERN_INFO "Bye Bye. Exiting netlink greetings kernel module\n");
	netlink_kernel_release(nl_sock);
}


module_init(netlink_greetings_init);
module_exit(netlink_greetings_exit);


MODULE_LICENSE("GPL");