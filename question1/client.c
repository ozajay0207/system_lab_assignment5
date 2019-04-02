#include <sys/socket.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <time.h>
#include <pthread.h>

#define PORT 8080 
#define TIMING_DATA 100
#define TIME_STRING_SIZE 100
#define USER_DATA_SIZE 100
#define BUFFER_SIZE 10000

//TO TAKE INPUT & CHECK THE AUTHENTICATION OF CLIENT CREDENTIALS
int check_auth(int sock){
	pthread_t tid;
	int read_val;
	char user_id[USER_DATA_SIZE];
	char password[USER_DATA_SIZE];
	char data_buffer[BUFFER_SIZE] =""; 

	//TAKE USERID AND PASSWORD FROM USER
	printf("Enter User Id:");
	scanf("%s",user_id);
	printf("Enter Password:");
	scanf("%s",password);

	//SEND THE CREDENTIALS TO SERVER FOR CHECKING
	strcpy(data_buffer,user_id);
	send(sock,data_buffer,BUFFER_SIZE,0);
	strcpy(data_buffer,password);
	send(sock,data_buffer,BUFFER_SIZE,0);
	strcpy(data_buffer,"");

	//CHECK FOR AUTHENTICATION REPLY
	read_val = read( sock , data_buffer, BUFFER_SIZE); 
	if(strcmp(data_buffer,"not_auth")==0){
		printf("Authentication Failed\n");
		return 0;
	}

	//PRINT THE AUTHENTICATED CLIENT
	printf("Client %s Authenticated\n",user_id);
	return 1;
}


//FUNCTION TO HANDLE ALL THE SERVICES EXCEPT THE UPLOAD SERVICE
void other_services(int sock){

	int read_val;
	char mssg_buffer[BUFFER_SIZE]="";
	char data_buffer[BUFFER_SIZE] =""; 
	char service_type[BUFFER_SIZE];
	char filename[500];
	FILE *fp;
	char file_buff[1024]="";

	//printf("Other Services\n");
	//LOOP UNTIL SERVER TELLS TO QUIT
	while(1){

		

		strcpy(data_buffer,"");
		read_val=read(sock,data_buffer,BUFFER_SIZE);	
		strcpy(service_type,data_buffer);

		//CASE TO PRINT THE MESSAGE SENT BY SERVER
		if(strcmp(service_type,"1")==0){
			strcpy(data_buffer,"");
			read_val=read(sock,data_buffer,BUFFER_SIZE);
			printf("%s",data_buffer);	

		//CASE TO READ INPUT FROM USER		
		}else if(strcmp(service_type,"2")==0){
			getchar();
			scanf("%[^\n]s",mssg_buffer);						
			send(sock,mssg_buffer,BUFFER_SIZE,0);
			

		//CASE TO RESEND THE RECEIVED MESSAGE STRING
		}else if(strcmp(service_type,"3")==0){

			strcpy(data_buffer,"");
			read_val=read(sock,data_buffer,BUFFER_SIZE);
			//printf("Received from Server:%s\n",data_buffer);		
			send(sock,data_buffer,BUFFER_SIZE,0);
			//printf("Sending it back...\n");

		//FOR ANY OTHER CASE BREAK
		}else if(strcmp(service_type,"4")==0){
			
			strcpy(data_buffer,"");
			read_val=read(sock,data_buffer,BUFFER_SIZE);
			strcpy(filename,data_buffer);
						
			//OPEN THE FILE TO UPLOAD TO SERVER
			fp=fopen(filename,"r");
			
		}else if(strcmp(service_type,"5")==0){
			fclose(fp);
		}else if(strcmp(service_type,"6")==0){
			if(fp!=NULL){		
				//SEND LINE BY LINE FILE DATA
				while (fgets(file_buff, sizeof(file_buff), fp) != NULL){
					send(sock , file_buff , BUFFER_SIZE , 0 ); 
				}	
				//SEND END OF FILE 
				strcpy(file_buff,"-999");
				send(sock , file_buff , BUFFER_SIZE , 0 ); 

			}else{
				printf("File not present\n");
				strcpy(data_buffer,"file not present");
				send(sock , data_buffer , BUFFER_SIZE , 0 ); 
				break;
			}
		}else{
			break;
		}
	}
}

int main(int argc, char const *argv[]) 
{ 

	int check=0;
	struct sockaddr_in address; 
	int sock = 0, read_val; 
	struct sockaddr_in serv_addr; 
	int choice;
	char choice_str[2];
	char data_buffer[BUFFER_SIZE] ="";
	char temp_buffer[BUFFER_SIZE]="";		

	//INTIALIZE THE CONNECTION AND SOCKETS
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 
	memset(&serv_addr, '0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0){ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 

	//CONNECT TO SERVER WHICH IS ACCPETING CONNECTIONS
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	}

	//AUTHENTICATE THE CLIENT AND CLOSE IF FAIL
	/*check = check_auth(sock);	
	if(check==0)
		return 0;
	*/
	 	
	//READ THE SERVICES AND DISPLAY IT TO USER
	 

	//LOOP UNTIL CLIENT QUITS
	while(1){

		strcpy(data_buffer,"");
		read_val = read( sock , data_buffer, BUFFER_SIZE);
	
		printf("\n\n%s\n",data_buffer); 
		//ASK FOR THE SERVICE TYPE
		printf("Enter a Choice:\n");		
		scanf("%d",&choice);

		read_val = read(sock,temp_buffer,BUFFER_SIZE);
		printf("MODIFIED:%s\n",temp_buffer);
		if(strcmp(temp_buffer,"$$$$$")==0){
			printf("Server Modified\n");
			continue;	
		}

		sprintf(choice_str, "%d", choice);
		send(sock , choice_str , BUFFER_SIZE , 0 ); 	
		
		if(choice==999){			
			printf("Exiting and Closing Thread...\n\n");
			break;
		}		
	
		//ALL THE CASE ARE DEFAULT EXCEPT FOR UPLOAD FILE
		switch(choice){			
			default:
				other_services(sock);
			break;
			
		}
	}	
	return 0; 
} 

