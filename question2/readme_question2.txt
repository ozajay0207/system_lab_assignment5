SYSTEM LAB : ASSIGNMENT 5

QUESTION 2 : RAW SOCKET

	Create your own version of data link layer neighbor discovery protocol using raw sockets using C/C++. Your implementation should be divided into a client and a server program.
	The client requests all of it’s link neighbors to advertise their HW address (MAC address) via ADVERTISE_REQUEST frame. The server program upon receiving an ADVERTISE_REQUEST 
	frame, identifies the receiving interface and replies back to the client with an ADVERTISE_REPLY frame. An ADVERTISEMENT_REPLY frame contains the MAC address of the receiving interface.
	Upon receiving ADVERTISEMENT_REPLY from all the interfaces, client program prints the neighbor MAC addresses.		

FILES AND FOLDERS :

	question2 folder contains the following : 

	files - contains the server.c and client.c files respectively.
	obj   - contains the output file for the client and server 
		(Run the make command to generate the output file in obj folder)


PROGRAM FLOW : 	
	
    SERVER
	
	1. Server waits for a advertise request frame from the client using the receive command on a raw socket which accepts all ethernet type of frames.
	
	2. It opens all ethernet packets and checks for ARP_REQUEST type.
	
	3. On successfully finding a ARP_REQUEST it opens the packet and extracts all the fields and prints appropriate fields and it saved the MAC and IP of the client.

	4. It forms a advertise response and forms a packet containing ARP_REPLY header along with ETHERNET frame.

	5. It opens the raw socket for server and sends it to client containing the required fields.

    CLIENT
	
	1. Client forms a advertise resquest and forms a packet containing ARP_REQUEST header along with ETHERNET frame containing the source IP and MAC ,
	   the destination IP is random and the destination MAC is of type broadcast.
	
	2. It opens the raw socket for client and sends it to all the connected link nodes.

	3. Client then waits for a advertise response frame from the all the connected nodes using the receive command on a raw socket which accepts all ethernet type of frames.
	
	4. It opens all ethernet packets and checks for ARP_RESPONSE type.
	
	5. On successfully receiving response it opens the packet and extracts all the fields and prints servers MAC .

	6. This socket is made open infinitely to accept response from all the nodes.


RUNNING THE PROGRAM : 
	
	1. Run the make file to generate the object files in obj folder.
	2. Run server first and then client(can be more than 1) using server and client files.
	3. Run the server multiple times before the client is made to run to receive the broadcast in multiple servers.

	Compilation SERVER (If you want to do other then MAKE):
			gcc server.c -o server -w

	Compilation CLIENT (If you want to do other then MAKE):
			gcc client.c -o client -w

	./server and ./client are required to run the server and client respectively.

