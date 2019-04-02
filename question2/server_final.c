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
 

uint8_t *src_mac, *dst_mac, *ethernet_frame;
uint8_t dest_mac[6],dest_ip[4];
char *interface, *src_ip,*target,recv_ip[100];

//DEFINING THE HEADER WITH THE REQUIRED FIELDS 
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
	uint8_t temp;
};
 
//DEFINING THE HEADER LENGTH AND OPCODE FOR ARP
#define ETHERNET_HDR_LEN 14      
#define IP_HDR_LEN 20   
#define ARP_HDR_LEN 29  
#define ARP_REQ_OP 1    
#define ARP_REPLY_OP 2
 

//WAIT FOR THE ADVERTISE REQUEST FROM CLIENT AND EXTRACTING THE CLIENT MAC AND IP FROM THE RECEIVED PACKET
int wait_for_request(){

	//DEFINING THE REQUIRED VARIABLES
	int i, sd, status;
	uint8_t *ethernet_frame;
	uint16_t ethernet_type;
	arp_hdr *arphdr;
 
	//ALLOCATING 65535 SIZE TO THE ETHERNET FRAME
	ethernet_frame = (uint8_t *) malloc (IP_MAXPACKET * sizeof (uint8_t));
 
	//CREATE A RAW SOCKET FOR RECEIVING THE FRAME
	if ((sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0) {
		perror("socket() failed ");
		return 1;
	}
 
	//DEFINE THE REFERENCE FOR ARP_HEADER STRUCTURE WITH APPROPRIATE LENGTH
	arphdr = ( arp_hdr *) (ethernet_frame + 6 + 6 + 2);

	//CONTINUE TO READ THE PACKET UNTIL ARP TYPE IS OBTAINED IN THE 12TH AND 13TH POSITION WHICH CONTAINS THE TYPE OF REQUEST

	//ethernet_type=((ethernet_frame[12]) << 8) + ethernet_frame[13];
	//while ((ethernet_type != ETH_P_ARP) || (ntohs (arphdr->opcode) != ARP_REPLY_OP)) {
	while (((((ethernet_frame[12]) << 8) + ethernet_frame[13]) != ETH_P_ARP) || (ntohs (arphdr->opcode) != ARP_REQ_OP) || (arphdr->temp != 1)) {

		//RECEIVE THE ETHERNET TYPE OF PACKET FROM THE RAW SOCKET
		status = recv (sd, ethernet_frame, IP_MAXPACKET, 0) ;

		//CHECK IF THERE IS AN ERROR ELSE CONTINUE
		if (status < 0){

	        	if (errno == EINTR) {
		 		memset (ethernet_frame, 0, IP_MAXPACKET * sizeof (uint8_t));
				continue; 
	        	} else {
				printf("Receiving the packet is unsuccessful");
				return 1;
	        	}
		}

        	//ethernet_type=((ethernet_frame[12]) << 8) + ethernet_frame[13];
	  }
	 	  
	//PRINTING THE MAC ADDRESSES OF SOURCE AND DESTINATION
	printf("\nExtracting ETHERNET Frame...\n");
	printf ("Source MAC:");
	for (i=6; i<12; i++) {
		if(i==11)
		printf ("%02x\n", ethernet_frame[i]);
		else
		printf ("%02x:", ethernet_frame[i]);
	}
	printf ("Destination MAC (Our MAC):");
	for (i=0; i<6; i++) {
		if(i==5)
		printf ("%02x\n", ethernet_frame[i]);
		else
		printf ("%02x:", ethernet_frame[i]);
	}
	
	printf("\nExtracting ARP Header...\n");
	//sprintf(target_mac,"%02x:%02x:%02x:%02x:%02x:%02x",arphdr->sender_mac[0],arphdr->sender_mac[1],arphdr->sender_mac[2],arphdr->sender_mac[3],arphdr->sender_mac[4],arphdr->sender_mac[5]);
	//printf("Target:%s\n",target_mac);

	//STORING CLIENT MAC FROM THE RECEIVED PACKET FOR ADVERTISE REPLY PURPOSE ( STORED IN GLOBAL VARIABLE )
	dest_mac[0] = arphdr->sender_mac[0];
	dest_mac[1] = arphdr->sender_mac[1];
	dest_mac[2] = arphdr->sender_mac[2];
	dest_mac[3] = arphdr->sender_mac[3];
	dest_mac[4] = arphdr->sender_mac[4];
	dest_mac[5] = arphdr->sender_mac[5];
	printf("MAC of Client:%02x:%02x:%02x:%02x:%02x:%02x\n",dest_mac[0],dest_mac[1],dest_mac[2],dest_mac[3],dest_mac[4],dest_mac[5]);

	//STORING CLIENT IP FROM THE RECEIVED PACKET FOR ADVERTISE REPLY PURPOSE ( STORED IN GLOBAL VARIABLE )
	dest_ip[0] = arphdr->sender_ip[0];
	dest_ip[1] = arphdr->sender_ip[1];
	dest_ip[2] = arphdr->sender_ip[2];
	dest_ip[3] = arphdr->sender_ip[3];
	printf("IP of Client:%u.%u.%u.%u\n",dest_ip[0],dest_ip[1],dest_ip[2],dest_ip[3]);

	//COPYING THE ADDRESS TO A STRING FOR ADDRESS RESOLUTION IN ADVERTISE REPLY STEP ( STORED IN GLOBAL VARIABLE )
	sprintf(recv_ip,"%u.%u.%u.%u",dest_ip[0],dest_ip[1],dest_ip[2],dest_ip[3]);	


	//RESET AND FREE THE MEMORY ALLOCATED TO THE ETHERNET FRAME 
	//memset (ethernet_frame, 0, IP_MAXPACKET * sizeof (uint8_t));
	free (ethernet_frame);
	return 0;
}

//FUNCTION TO SEND THE UNICAST REPLY CONTAINING SERVER MAC AND IP TO CLIENT 
int advertise_reply(){

	int  ret;
	int i, status, frame_length, sd, bytes;  
	arp_hdr arphdr;

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

	//SET THE IP ADDRESS OF THE SERVER 
	strcpy (src_ip, "192.168.0.102");
	 
	//SET THE IP ADDRESS OF CLIENT FROM THE GLOBAL VARIABLE SET USING THE RECEIVED REQUEST
	strcpy (target, recv_ip);
	 
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
		printf("Failed to get the hardware address\n");
		return 1;
	}
	close (sd);
	 
	//COPY THE SERVER'S MAC ADDRESS TO THE HEADER FIELD
	memcpy (src_mac, ifr.ifr_hwaddr.sa_data, 6 * sizeof (uint8_t));
	 
	//PRINTING THE MAC ADDRESS OF THE SERVER
	//printf ("MAC address for interface %s is ", interface);
	/*for (i=0; i<6; i++) {
		if(i==5)
		printf ("%02x\n", src_mac[i]);
		else
		printf ("%02x:", src_mac[i]);
	}*/

	//SET THE DESTINATION MAC ADDRESS AS A BROADCAST
	//memset (dst_mac, 0xff, 6 * sizeof (uint8_t));
	dst_mac[0] = dest_mac[0]; 
	dst_mac[1] = dest_mac[1]; 
	dst_mac[2] = dest_mac[2]; 
	dst_mac[3] = dest_mac[3]; 
	dst_mac[4] = dest_mac[4]; 
	dst_mac[5] = dest_mac[5];
	  
	//GET THE INDEX OF THE INTERFACE USING THE NAME OF INTERFACE
	memset (&device, 0, sizeof (device));
	if ((device.sll_ifindex = if_nametoindex (interface)) == 0) {
		printf("if_nametoindex() failed to obtain interface index\n");
		return 1;
	}
	//printf ("Index for interface %s is %i\n", interface, device.sll_ifindex);
	 
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

	//memcpy (device.sll_addr, src_mac, 6 * sizeof (uint8_t));
	
	 
	//SETTING THE HEADER FIELD DATA
	arphdr.h_type = htons (1);	
	arphdr.p_type = htons (ETH_P_IP);	 
	arphdr.header_len = 6;	 
	arphdr.p_len = 4;
	arphdr.temp = 1;
	arphdr.opcode = htons (ARP_REPLY_OP);	
	memcpy (&arphdr.sender_mac, src_mac, 6 * sizeof (uint8_t));
	
	//COPY THE CLIENT'S IP WHICH WAS STORED DURING THE REQUEST
	arphdr.target_ip[0] = dest_ip[0];
	arphdr.target_ip[1] = dest_ip[1];
	arphdr.target_ip[2] = dest_ip[2];
	arphdr.target_ip[3] = dest_ip[3];
	device.sll_halen = 6;

	//SETTING THE TARGET MAC ADDRESS OF THE CLIENT WHICH WAS STORED DURING THE REQUEST
	arphdr.target_mac[0] = dest_mac[0];
	arphdr.target_mac[1] = dest_mac[1];
	arphdr.target_mac[2] = dest_mac[2];
	arphdr.target_mac[3] = dest_mac[3];
	arphdr.target_mac[4] = dest_mac[4];
	arphdr.target_mac[5] = dest_mac[5];

	//STORING THE LENGTH OF FRAME TO BE SENT
	frame_length = 6 + 6 + 2 + ARP_HDR_LEN;
	 
	//SETTING THE FIELDS OF THE FRAME APPROPRIATELY
	memcpy (ethernet_frame, dst_mac, 6 * sizeof (uint8_t));
	memcpy (ethernet_frame + 6, src_mac, 6 * sizeof (uint8_t));
	ethernet_frame[12] = ETH_P_ARP / 256;
	ethernet_frame[13] = ETH_P_ARP % 256;
	memcpy (ethernet_frame + ETHERNET_HDR_LEN, &arphdr, ARP_HDR_LEN * sizeof (uint8_t));
	 
	//OPEN THE SOCKET TO SEND THE ADVERTISE REPONSE BACK TO CLIENT CONTAINING THE MAC AND IPs
	sd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL));
	if (sd < 0) {
		perror("socket() failed ");
		return 1;
	}

	printf("Sending to : %s",recv_ip);
        printf(" with mac :%02x:%02x:%02x:%02x:%02x:%02x\n",dest_mac[0],dest_mac[1],dest_mac[2],dest_mac[3],dest_mac[4],dest_mac[5]);	 

	//SEND THE FRAME USING THE RAW SOCKET BACK TO CLIENT
	bytes = sendto (sd, ethernet_frame, frame_length, 0, (struct sockaddr *) &device, sizeof (device));	

	printf("Sent Successfully\n");
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


//MAIN FUNCTION TO HANDLE THE SERVER PROGRAM
int main (int argc, char **argv)
{

	int ret;

	//THE SERVER WAITS ON RECEIVE CALL TO RECEIVE A RAW SOCKET FROM CLIENT CONTAINING THE ADVERTISE REQUEST MESSAGE
	printf("\nWaiting for advertise frame from client ...\n");
	ret = wait_for_request();
    	
	if(ret==0){
		//USED FOR SYNCHRONIZING CLIENT TO COMPLETE THE BROADCAST AND ENTER THE RECEIVE MODE
		sleep(1);

		//FORMING THE PACKET CONTAINING MAC AND SENDING IT CLIENT USING RAW SOCKET
		printf("\nSending Advertise Response\n");
		advertise_reply();    
	}else{
		printf("Failed to get the advertise request from the client : check for errors\n");
	}


	return 0;
}
 

