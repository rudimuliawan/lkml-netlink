//
// Created by Rudi Muliawan on 23/10/21.
//

#include <errno.h>
#include <linux/netlink.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "netlink_utils.h"


typedef struct thread_arg_{
    int sock_fd;
} thread_arg_t;


static void exit_userspace();
static void * _start_kernel_data_receiver_thread(void *arg);
void start_kernel_data_receiver_thread(thread_arg_t *thread_arg);
static void greet_kernel(int sock_fd, char *msg, uint32_t msg_len);
int send_netlink_message_to_kernel(
    int sock_fd, char *msg, uint32_t msg_size, int nlmsg_type
);
uint32_t new_seq_no();


static void exit_userspace(int sock_fd)
{
    close(sock_fd);
    exit(0);
}


uint32_t new_seq_no(){
    static uint32_t seq_no = 0 ;
    return seq_no++;
}


static void *_start_kernel_data_receiver_thread(void *arg)
{
    int rc = 0;
    struct iovec iov;
    struct nlmsghdr *nlh_recv = NULL;
    static struct msghdr outermsghdr;
    int sock_fd = 0;

    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    sock_fd = thread_arg->sock_fd;

    do {
        memset(nlh_recv, 0, NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD));

        iov.iov_base = (void *) nlh_recv;
        iov.iov_len = NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD);

        memset(&outermsghdr, 0, sizeof(struct msghdr));

        outermsghdr.msg_iov     = &iov;
        outermsghdr.msg_name    = NULL;
        outermsghdr.msg_iovlen  = 1;
        outermsghdr.msg_namelen = 0;

        rc = recvmsg(sock_fd, &outermsghdr, 0);

        printf("Received Netlink msg from kernel, bytes recvd = %d\n", rc);
        nlmsg_dump(outermsghdr.msg_iov->iov_base);
    } while (1);
}


void start_kernel_data_receiver_thread(thread_arg_t *thread_arg)
{
    pthread_attr_t attr;
    pthread_t recv_pkt_thread;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(
        &recv_pkt_thread, &attr,
        _start_kernel_data_receiver_thread,
        (void *) thread_arg
    );
}


static void greet_kernel(int sock_fd, char *msg, uint32_t msg_len)
{
    send_netlink_message_to_kernel(sock_fd, msg, msg_len, NLMSG_GREET);
}


int send_netlink_message_to_kernel(
    int sock_fd, char *msg, uint32_t msg_size,
    int nlmsg_type
)
{
    struct sockaddr_nl src_addres, dest_address;
    struct nlmsghdr *nlh = NULL;

    memset(&src_addres, 0, sizeof(src_addres));
    src_addres.nl_family = AF_NETLINK;
    src_addres.nl_pid = getpid();

    memset(&dest_address, 0, sizeof(dest_address));
    dest_address.nl_family = AF_NETLINK;
    dest_address.nl_pid = 0;

    nlh = (struct nlmsghdr *) calloc(1, NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD));

    nlh->nlmsg_len = NLMSG_HDRLEN + NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags |= NLM_F_REQUEST;;
    nlh->nlmsg_type = nlmsg_type;
    nlh->nlmsg_seq = new_seq_no();

    strncpy(NLMSG_DATA(nlh), msg, msg_size);

    struct iovec iov;
    iov.iov_base = (void *) nlh;
    iov.iov_len = nlh->nlmsg_len;

    static struct msghdr outermsghdr;
    memset(&outermsghdr, 0, sizeof(struct msghdr));
    outermsghdr.msg_name = (void *) &dest_address;
    outermsghdr.msg_namelen = sizeof(dest_address);
    outermsghdr.msg_iov = &iov;
    outermsghdr.msg_iovlen = 1;

    int rc = sendmsg(sock_fd, &outermsghdr, 0);
    if (rc < 0) {
        printf("Message sending failed, error no %d\n", errno);
    }

    return rc;
}


int main(int argc, char **argv)
{
    int choice;
    int sock_fd;

    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST_PROTOCOL);
    if (sock_fd < 0) {
        printf("Error : Netlink socket creation failed; Error = %d\n", errno);
        exit(EXIT_FAILURE);
    }

    thread_arg_t thread_arg;
    thread_arg.sock_fd = sock_fd;

    // start_kernel_data_receiver_thread(&thread_arg);

    while (1) {
        printf("Main Menu:\n");
        printf("1. Greet Kernel\n");
        printf("2. Adding Route\n");
        printf("3. Exit\n");
        printf("choice? ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                {
                    char user_msg[MAX_PAYLOAD] = {'H', 'E', 'L', 'L', 'O', 'U', 'S', 'E', 'R', 'S', 'P', 'A', 'C', 'E', '\n'};

                    /*
                    if ((fgets((char *) user_msg, MAX_PAYLOAD - 1, stdin) == NULL)) {
                        printf("error in reading from stdin\n");
                        exit(EXIT_SUCCESS);
                    }
                    */

                   reet_kernel(sock_fd, user_msg, strlen(user_msg));
                }
                break;

            case 2:
                break;

            case 3:
                exit_userspace(sock_fd);
                break;

            default:
                ;
        }
    }

    return 0;
}