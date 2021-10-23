//
// Created by Rudi Muliawan on 23/10/21.
//

#ifndef LKML_NETLINK_NETLINK_UTILS_H
#define LKML_NETLINK_NETLINK_UTILS_H

#include <linux/netlink.h>

#define MAX_PAYLOAD 1024
#define NLMSG_GREET 20
#define NETLINK_TEST_PROTOCOL 31

static inline char * netlink_get_msg_type(__u16 nlmsg_type){
    switch(nlmsg_type){
        case NLMSG_NOOP:
            return "NLMSG_NOOP";
        case NLMSG_ERROR:
            return "NLMSG_ERROR";
        case NLMSG_DONE:
            return "NLMSG_DONE";
        case NLMSG_OVERRUN:
            return "NLMSG_OVERRUN";
        case NLMSG_GREET:
            return "NLMSG_GREET";
        default:
            return "NLMSG_UNKNOWN";
    }
}

static inline void nlmsg_dump(struct nlmsghdr *nlh){
    #ifdef __KERNEL__
        printk(KERN_INFO "Dumping Netlink Msgs Hdr");
    #else
        printf("Dumping Netlink Msgs Hdr");
    #endif

    #ifdef __KERNEL__
        printk(KERN_INFO " Netlink Msg Type = %s",
                netlink_get_msg_type(nlh->nlmsg_type));
    #else
        printf(" Netlink Msg Type = %s",
               netlink_get_msg_type(nlh->nlmsg_type));
    #endif

    #ifdef __KERNEL__
        printk(KERN_INFO " Netlink Msg len  = %d", nlh->nlmsg_len);
    #else
        printf(" Netlink Msg len  = %d", nlh->nlmsg_len);
    #endif

    #ifdef __KERNEL__
        printk(KERN_INFO " Netlink Msg flags  = %d", nlh->nlmsg_flags);
    #else
        printf(" Netlink Msg flags  = %d", nlh->nlmsg_flags);
    #endif

    #ifdef __KERNEL__
        printk(KERN_INFO " Netlink Msg Seq#  = %d", nlh->nlmsg_seq);
    #else
        printf(" Netlink Msg Seq#  = %d", nlh->nlmsg_seq);
    #endif

    #ifdef __KERNEL__
        printk(KERN_INFO " Netlink Msg Pid#  = %d", nlh->nlmsg_pid);
    #else
        printf(" Netlink Msg Pid#  = %d", nlh->nlmsg_pid);
    #endif
}

#endif // LKML_NETLINK_NETLINK_UTILS_H
