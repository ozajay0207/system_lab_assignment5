#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>          
#include <string.h>          
#include <netdb.h>           
#include <sys/types.h>       
#include <sys/socket.h>      
#include <sys/ioctl.h>       
#include <bits/ioctls.h>     
#include <net/if.h>          
#include <netinet/in.h>      
#include <netinet/ip.h>      
#include <arpa/inet.h>       
#include <linux/if_ether.h>  
#include <linux/if_packet.h> 
#include <net/ethernet.h>
#include <errno.h>      
 
//DEFINING THE HEADER FOR THE RAW SOCKET 
typedef struct _arp_hdr arp_hdr;
struct _arp_hdr {
  uint16_t h_type;
  uint16_t p_type;
  uint8_t header_len;
  uint8_t p_len;
  uint16_t opcode;
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t target_mac[6];
  uint8_t target_ip[4];
};
 
//DEFINING THE HEADER LENGTH AND OPCODE FOR ARP
#define ETHERNET_HEADER_LEN 14      
#define IP_HDR_LEN 20   
#define ARP_HEADER_LEN 28  
#define ARP_REQ_OP 1    
#define ARP_REPLY_OP 2
 
//SEND THE ADVERTISE REQUEST FRAME TO THE GIVEN IP USING RAW SOCKET
int advertise_request(char IP[]){

	int ret;
	int i, status, frame_length, sd, bytes;
	char *interface, *target, *src_ip;
	arp_hdr arphdr;
	uint8_t *src_mac, *dst_mac, *ethernet_frame;
	struct addrinfo hints, *res;
	struct sockaddr_in *ipv4;
	struct sockaddr_ll device;
	struct ifreq ifr;
 
	// ALLOCATE APPROPRIATE MEMORY TO THE HEADER FIELDS
	src_mac = (uint8_t *) malloc (6 * sizeof (uint8_t));
	dst_mac = (uint8_t *) malloc (6 * sizeof (uint8_t));
	interface = (char *) malloc (40 * sizeof (char));
	target = (char *) malloc (40 * sizeof (char));
	src_ip = (char *) malloc (INET_ADDRSTRLEN * sizeof (char)) ;
	ethernet_frame = (uint8_t *) malloc ((IP_MAXPACKET + 1) * sizeof (uint8_t));
 
	//SETTING THE SYSTEM INTERFACE
	strcpy (interface, "wlp5s0");

	// Set destination MAC address: broadcast address
	memset (dst_mac, 0xff, 6 * sizeof (uint8_t));
	 
	//SET THE IP ADDRESS OF THE SLIENT
	strcpy (src_ip, "192.168.0.102");
 
	//COPY THE IP FROM THE PARAMETER OF FUNCTION
	strcpy (target, IP);
	 
	//CREATE A SD FOR GETTING THE INTERFACE OF THE SERVER
	if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
		perror ("Unable to get SD for ioctl");
		return 1;
	}
 
	memset (&ifr, 0, sizeof (ifr));
	snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", interface);
	
	//GETTING THE MAC ADDRESS OF THE INTERFACE (i.e. MAC of the system)
	ret = ioctl (sd, SIOCGIFHWADDR, &ifr); 
	if (ret < 0) {
		printf("Failed to get interface\n");
		return 1;
	}
	close (sd);
 
	//COPY THE SERVER'S MAC ADDRESS TO THE HEADER FIELD
	memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));
	 
	//PRINTING THE MAC ADDRESS OF THE SERVER
	printf ("MAC address for interface %s is ", interface);
	for (i=0; i<6; i++) {
		if(i==5)
		printf ("%02x\n", src_mac[i]);
		else
		printf ("%02x:", src_mac[i]);
	}
 
	//GET THE INDEX OF THE INTERFACE USING THE NAME OF INTERFACE	
	memset (&device, 0, sizeof (device));
	if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
		printf("if_nametoindex() failed to obtain interface index\n");
		return 1;
	}
	printf ("Index for interface %s is %i\n", interface, device.sll_ifindex);
	 	
 
	//SETTING HINTS
	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = hints.ai_flags | AI_CANONNAME;

	//CHECK FOR SOURCE IP 
	ret = inet_pton (AF_INET, src_ip, &arphdr.sender_ip);
	if (ret != 1) {
		fprintf (stderr, "inet_pton() failed for source IP address.\nError message: %s", strerror (status));
		return 1;
	}
	 
	//CHECK FOR TARGET IP 
	ret = getaddrinfo (target, NULL, &hints, &res);
	if (ret != 0) {
		fprintf (stderr, "getaddrinfo() failed: %s\n", gai_strerror (status));
		return 1;
	}

	ipv4 = (struct sockaddr_in *) res->ai_addr;
	memcpy (&arphdr.target_ip, &ipv4->sin_addr, 4 * sizeof (uint8_t));
	freeaddrinfo (res);
 
	// Fill out sockaddr_ll.
	device.sll_family = AF_PACKET;

	memcpy (device.sll_addr, src_mac, 6 * sizeof (uint8_t));
	device.sll_halen = 6;
 
	//SETTING THE HEADER DATA
	arphdr.h_type = htons (1);	
	arphdr.p_type = htons (ETH_P_IP);	 
	arphdr.header_len = 6;	 
	arphdr.p_len = 4;
	arphdr.opcode = htons (ARP_REQ_OP);	
	memcpy (&arphdr.sender_mac, src_mac, 6 * sizeof (uint8_t));

 
	// Ethernet frame length = ethernet header (MAC + MAC + ethernet type) + ethernet data (ARP header)
	frame_length = 6 + 6 + 2 + ARP_HEADER_LEN;
	 
	//SET THE FIELDS OF THE FRAME APPROPRIATELY
	memcpy (ethernet_frame, dst_mac, 6 * sizeof (uint8_t));
	memcpy (ethernet_frame + 6, src_mac, 6 * sizeof (uint8_t));	 
	ethernet_frame[12] = ETH_P_ARP / 256;
	ethernet_frame[13] = ETH_P_ARP % 256;
	memcpy (ethernet_frame + ETHERNET_HEADER_LEN, &arphdr, ARP_HEADER_LEN * sizeof (uint8_t));
	 
	// Submit request for a raw socket descriptor.
	sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL));
	if (sd < 0) {
		perror("socket() failed ");
		return 1;
	}
printf("ARP::::%u.%u.%u.%u\n",arphdr.sender_ip[0],arphdr.sender_ip[1],arphdr.sender_ip[2],arphdr.sender_ip[3]);
	 
	//SEND THE REQUEST FRAME TO THE IP MENTIONED 
	bytes = sendto (sd, ethernet_frame, frame_length, 0, (struct sockaddr *) &device, sizeof (device));	
	close (sd);
	 
	//FREE THE ALLOCATED MEMORY
	free (src_mac);
	free (dst_mac);
	free (ethernet_frame);
	free (interface);
	free (target);
	free (src_ip);

	return 0;

}

//WAIT FOR THE REPLE FROM ANY CONNECT NEIGHBOUR CONTAINING THE MAC ADDRESS OF ITSELF
int wait_for_reply(){

	int i, sd, status;
	uint8_t *ethernet_frame;
	arp_hdr *arphdr;
 
	//ALLOCATING 65535 SIZE TO THE ETHERNET FRAME
	ethernet_frame = (uint8_t *) malloc (IP_MAXPACKET * sizeof (uint8_t));
 
	//CREATE A RAW SOCKET FOR RECEIVING THE FRAME
	if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
		perror("socket() failed ");
		return 1;
	}
 
	//DEFINE THE REFERENCE FOR ARP_HEADER STRUCTURE WITH APPROPRIATE LENGTH
	arphdr = (arp_hdr *) (ethernet_frame + 6 + 6 + 2);

	//WAIT INFINITELY FOR THE REQUEST
	while(1)
	{

		//CONTINUE TO READ THE PACKET UNTIL ARP TYPE IS OBTAINED IN THE 12TH AND 13TH POSITION WHICH CONTAINS THE TYPE OF REQUEST
		while (((((ethernet_frame[12]) << 8) + ethernet_frame[13]) != ETH_P_ARP) || (ntohs (arphdr->opcode) != ARP_REPLY_OP)) {

			//RECEIVE THE ETHERNET TYPE OF PACKET FROM THE RAW SOCKET
			status = recv (sd, ethernet_frame, IP_MAXPACKET, 0) ;

			//CHECK IF THERE IS AN ERROR
			if (status < 0){
				if (errno == EINTR) {
			 		memset (ethernet_frame, 0, IP_MAXPACKET * sizeof (uint8_t));
					continue;  
				} else {
					printf("Receiving the packet is unsuccessful");
					return 1;
				}
			}
	  	}

		//PRINTING THE MAC ADDRESSES OF SOURCE AND DESTINATION
		printf ("Source MAC: ");
		for (i=6; i<12; i++) {
			if(i==11)
			printf ("%02x\n", ethernet_frame[i]);
			else
			printf ("%02x:", ethernet_frame[i]);
		}
		printf ("Destination MAC : ");
		for (i=0; i<6; i++) {
			if(i==5)
			printf ("%02x\n", ethernet_frame[i]);
			else
			printf ("%02x:", ethernet_frame[i]);
		}

		//RESET THE MEMORY ALLOCATED TO ETHERNET FRAME FOR HANDLING THE RESPONSE FROM NEXT IP ADDRESS
		memset (ethernet_frame, 0, IP_MAXPACKET * sizeof (uint8_t));
	} 

	//FREE THE ALLOCATED MEMORY
	free (ethernet_frame);
	 
	return 0;
}
 
//MAIN FUNCTION TO HANDLE THE CLIENT PROGRAM
int main (int argc, char **argv)
{
	int ret;
	char temp[4];
	char neighbour_ip[500];
	int i;

	//SEND THE BROADCAST REQUEST PACKET TO ALL THE IP ADDRESS RANGING FROM (192.168.0.1-254)
	for(i=4;i<5;i++){

		//SET UP THE DESTINATION IP
		strcpy(neighbour_ip,"192.168.0.");
		sprintf(temp,"%d",i);
		strcat(neighbour_ip,temp);
		printf("IP:%s\n",neighbour_ip);

		//SEND THE ADVERTISE REQUEST FRAME USING THE RAW PACKET TO GIVEN IP ADDRESS
		ret=advertise_request(neighbour_ip);

		if(ret!=0){
			printf("Unable to send the packet to : %s\n",neighbour_ip);
		}
	}

	//WAIT FOR REPLY FROM ALL THE NEIGHBOURS WHO RECEIVED THE REQUEST
	wait_for_reply();

	return 0;	
}
 
