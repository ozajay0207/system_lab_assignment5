#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>

static int s = -1;
static int s1 = -1;

void onexit(int signum)
{
    (void)signum;
    printf("Exiting");
    close(s);
    close(s1);
}

int main()
{
    char buf[1600];
    ssize_t recv_size = -1;
    ssize_t send_size = -1;

    int i = 0;

    struct sockaddr_ll socket_address, socket_address1;

    s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    s1 = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if ((s == -1) || (s1 == -1))
    {
        perror("Socket creation failed");
        exit (0);
    }

    signal(SIGINT, onexit);

    memset(&socket_address, 0, sizeof (socket_address));
    socket_address.sll_family = PF_PACKET;
    socket_address.sll_ifindex = if_nametoindex("eth0");
    socket_address.sll_protocol = htons(ETH_P_ALL);

    i = bind(s, (struct sockaddr*)&socket_address,
    sizeof(socket_address));
    if (i == -1)
    {
        perror("Bind");
        exit (0);
    }

    memset(&socket_address1, 0, sizeof (socket_address1));
    socket_address1.sll_family = PF_PACKET;
    socket_address1.sll_ifindex = if_nametoindex("eth1");
    socket_address1.sll_protocol = htons(ETH_P_ALL);

    i = bind(s1, (struct sockaddr*)&socket_address1,
    sizeof(socket_address1));
    if (i == -1)
    {
        perror("Bind");
        exit (0);
    }

    while (1)
    {
        memset(&buf, 0, sizeof(buf));

        recv_size = recv(s, &buf, sizeof(buf), 0);
        if (recv_size == -1)
        {
            perror("Socket receive");
            exit (0);
        }

        printf("\n");
        for(i=0; i < recv_size; i++)
        {
            printf("%02x ", buf[i]);
        }

        send_size = send(s1, &buf, recv_size, 0);
        if (send_size == -1)
        {
            perror("Socket send");
            exit (0);
        }
    }

    return 0;
}
