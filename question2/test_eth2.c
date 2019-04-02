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


int main(){
	int fd=socket(AF_PACKET,SOCK_DGRAM,htons(ETH_P_ARP));
	if (fd==-1) {
	    die("%s",strerror(errno));
	}

	struct ifreq ifr;
	size_t if_name_len=strlen(if_name);
	if (if_name_len<sizeof(ifr.ifr_name)) {
	    memcpy(ifr.ifr_name,if_name,if_name_len);
	    ifr.ifr_name[if_name_len]=0;
	} else {
	    die("interface name is too long");
	}
	if (ioctl(fd,SIOCGIFINDEX,&ifr)==-1) {
	    die("%s",strerror(errno));
	}
	int ifindex=ifr.ifr_ifindex;

	const unsigned char ether_broadcast_addr[]=
	    {0xff,0xff,0xff,0xff,0xff,0xff};

	struct sockaddr_ll addr={0};
	addr.sll_family=AF_PACKET;
	addr.sll_ifindex=ifindex;
	addr.sll_halen=ETHER_ADDR_LEN;
	addr.sll_protocol=htons(ETH_P_ARP);
	memcpy(addr.sll_addr,ether_broadcast_addr,ETHER_ADDR_LEN);

	struct ether_arp req;
	req.arp_hrd=htons(ARPHRD_ETHER);
	req.arp_pro=htons(ETH_P_IP);
	req.arp_hln=ETHER_ADDR_LEN;
	req.arp_pln=sizeof(in_addr_t);
	req.arp_op=htons(ARPOP_REQUEST);
	memset(&req.arp_tha,0,sizeof(req.arp_tha));

	const char* target_ip_string="192.168.0.83";
	struct in_addr target_ip_addr={0};
	if (!inet_aton(target_ip_string,&target_ip_addr)) {
	    die("%s is not a valid IP address",target_ip_string);
	}
	memcpy(&req.arp_tpa,&target_ip_addr.s_addr,sizeof(req.arp_tpa));

	if (sendto(fd,&req,sizeof(req),0,(struct sockaddr*)&addr,sizeof(addr))==-1) {
	    die("%s",strerror(errno));
	}

	return 0;
}




