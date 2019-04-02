#include<stdio.h>
#include<string.h>
#include<malloc.h>
#include<errno.h>

#include<sys/socket.h>
#include<sys/types.h>
#include<sys/ioctl.h>

#include<net/if.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/if_ether.h>
#include<netinet/udp.h>
#include<stdlib.h>
#include<linux/if_packet.h>

#include<arpa/inet.h>


int main(){
	int i;
	int sock_raw;
	char interface[50];
	uint8_t *src_mac, *dst_mac, *ether_frame;  
	struct sockaddr_ll device;


	memset(src_mac,0,sizeof(src_mac));

	sock_raw=socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW);
	if(sock_raw == -1)
	printf("error in socket");

	strcpy (interface, "wlp5s0");
	printf("Interface:%s\n",interface);

	struct ifreq ifreq_i;
	memset(&ifreq_i,0,sizeof(ifreq_i));
	strncpy(ifreq_i.ifr_name,interface,IFNAMSIZ-1); //giving name of Interface
	 
	if((ioctl(sock_raw,SIOCGIFINDEX,&ifreq_i))<0)
	printf("error in index ioctl reading");//getting Index Name
	 
	printf("index=%d\n",ifreq_i.ifr_ifindex);
//	printf("index=%s\n",ifreq_i.ifr_name);

	struct ifreq ifreq_c;
	memset(&ifreq_c,0,sizeof(ifreq_c));
	strncpy(ifreq_c.ifr_name,interface,IFNAMSIZ-1);//giving name of Interface
	 
	if((ioctl(sock_raw,SIOCGIFHWADDR,&ifreq_c))<0) //getting MAC Address
	printf("error in SIOCGIFHWADDR ioctl reading");
	else{
	printf("success\n");	
	}
	

	memcpy (src_mac, ifreq_c.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));
	 
	// Report source MAC address to stdout.
	printf ("MAC address for interface %s is ", interface);
	for (i=0; i<5; i++) {
	    printf ("%02x:", src_mac[i]);
	}
	printf ("%02x\n", src_mac[5]);
 

	memset (&device, 0, sizeof (device));
	if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
	    perror ("if_nametoindex() failed to obtain interface index ");
	    exit (EXIT_FAILURE);
	}
	printf ("Index for interface %s is %i\n", interface, device.sll_ifindex);
 
	return 0;

}
