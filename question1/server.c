#include <stdio.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <string.h> 
#include <pthread.h> 
#include <time.h>
#include <unistd.h> 
#include <sys/stat.h>
#include <sys/types.h>

#define NO_OF_USERS 3
#define NO_OF_SERVICES 5
#define TIMING_DATA 100
#define TIME_STRING_SIZE 100
#define USER_DATA_SIZE 100
#define BUFFER_SIZE 10000
#define PORT 8080 
#define FILENAME_SIZE 500

//TO CALCULATE TIME
int gettimeofday(struct timeval *,struct timezone *);
int socketList[100];
int modified_flag=0;

char first[]="$$$$$";
char path[] = "server";

char lastTime[100],currentTime[100]="Nothing";
int count = 0;
int flag=0;

void *getFileCreationTime(void *varg) {
    
    while(1)
    {
	    int bytes;
	    char data_buffer[BUFFER_SIZE];
	    struct stat attr;
    	    stat(path, &attr);
	    strcpy(lastTime,currentTime);
	    strcpy(currentTime,ctime(&attr.st_ctime));
	    //sprintf(currentTime,"%s",ctime(&attr.st_mtime));
	    sleep(2);
	    if(strcmp(lastTime,currentTime) != 0 )
		{
			if(flag==0)
			{
				flag+=1;
			}
			else
			{
				modified_flag=1;
				printf("Time Difference : %d\n",strcmp(lastTime,currentTime));
				printf("Last modified time : %s", lastTime);
		    		printf("Current modified time : %s", currentTime);
				char argumentsToSend[100];				
				char *check_command[10],temp_array[10][10],str[10]="server";
				check_command[0] = str;
				for(int i = 0; i< count; i++)
				{
					sprintf(temp_array[i],"%d", socketList[i]);
					check_command[i+1] = temp_array[i];
					strcpy(data_buffer,"$$$$$");
					printf("Socket To send:%d\n",socketList[i]);
					bytes=send(socketList[i],data_buffer,BUFFER_SIZE,0);
					printf("Bytes Sent:%d\n",bytes);
				}
				check_command[count+1] = NULL;
				for(int i=0;i<=count;i++)
					printf("Check Command :%s\n",check_command[i]);
				execv("./server",check_command);
				printf("EXECV command not executed\n"); 
			}
		}
    }
}


//STRUCTURE FOR USER 
struct credentials{
	char user_id[USER_DATA_SIZE];
	char password[USER_DATA_SIZE];
}users[NO_OF_USERS];

//INITIALIZE THE USER AND THEIR CREDENTIALS
void initialize_user_credentials(struct credentials users[]){
	strcpy(users[0].user_id,"jay");
	strcpy(users[0].password,"jay123");
	strcpy(users[1].user_id,"harsh");
	strcpy(users[1].password,"jay123");
	strcpy(users[2].user_id,"monika");
	strcpy(users[2].password,"jay123");
}

//INITIALIZE THE SERVICES TO OFFER THE CLIENTS
void initialize_services(char services[]){

	char service_list[NO_OF_SERVICES][200];
	strcpy(service_list[0],"1.Echo Request-Reply");
	strcpy(service_list[1],"2.Application Layer RTT Measurement");
	strcpy(service_list[2],"3.File Upload");
	strcpy(service_list[4],"999.Done");
	strcpy(service_list[3],"4.Add 2 numbers");

	//COPY THE SERVICES TO A COMMON MESSAGE STRING
	strcpy(services,"\n**************** SELECT FROM GIVEN SERVICES ****************\n");
	for(int i=0;i<NO_OF_SERVICES;i++){
		strcat(services,"\n");
		strcat(services,service_list[i]);
	}

}

//SERVICE 1 : ECHO REQUEST REPLY 
void echo_request_reply(int new_socket,int client_id){

	int read_val;
	int counter=0;
	char mssg_buffer[BUFFER_SIZE]="";
	char data_buffer[BUFFER_SIZE] =""; 
	char input_string[BUFFER_SIZE]="";
	char timings[TIMING_DATA][TIME_STRING_SIZE];
	struct timeval start, stop;
	double secs = 0;
		
	//LOOP UNTIL CLIENT SENDS 'N' FOR ECHO MESSAGE
	while(1){

		//TAKE INPUT STRING FROM USER VIA CLIENT
		strcpy(data_buffer,"1");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		strcpy(data_buffer,"Enter Input String:");
		send(new_socket,data_buffer,BUFFER_SIZE,0);

		//READ THE INPUT STRING
		strcpy(data_buffer,"2");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		strcpy(data_buffer,"");
		read_val = read( new_socket , data_buffer, BUFFER_SIZE ); 
		printf("\nClient %d : Received Input String from User:%s\n",client_id,data_buffer);
		strcpy(input_string,data_buffer);

		//INIT THE TIMER	
		gettimeofday(&start, NULL);

		//SEND-RECEIVE THE STRING (1 ROUND)
		strcpy(data_buffer,"3");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		printf("Client %d : Sending to Client:%s\n",client_id,input_string);
		send(new_socket,input_string,BUFFER_SIZE,0);
		strcpy(data_buffer,"");
		read_val = read( new_socket , data_buffer, BUFFER_SIZE ); 
		printf("Client %d : Received Back:%s\n",client_id,data_buffer);
		
		//STOP THE TIMER
		gettimeofday(&stop, NULL);

		//CALCULATE THE TIME IN MILLISECONDS
		secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);

		//SAVE THE TIMER FOR CURRENT ROUND
		strcpy(data_buffer,"");
		sprintf(data_buffer,"%f",secs);
		strcpy(timings[counter++],data_buffer);
		
		//ASK FOR QUIT FROM USER
		strcpy(data_buffer,"1");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		strcpy(data_buffer,"Do you want to continue (y/n):");
		send(new_socket,data_buffer,BUFFER_SIZE,0);

		//CHECK THE CLIENT RESPONSE
		strcpy(data_buffer,"2");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		strcpy(data_buffer,"");
		read_val = read( new_socket , data_buffer, BUFFER_SIZE ); 
		printf("Client %d : Client Continuation Choice:%s\n",client_id,data_buffer);

		//BREAK IF CHOICE IS 'N' AND SEND THE TIME FOR EVERY ROUND TO CLIENT TO PRINT
		if(strcmp(data_buffer,"n")==0){
			strcpy(data_buffer,"1");
			send(new_socket,data_buffer,BUFFER_SIZE,0);
			strcpy(data_buffer,"\nRound : Time Taken\n");
			send(new_socket,data_buffer,BUFFER_SIZE,0);
			for(int i=0;i<counter;i++){
				//printf("%d : %s\n",(i+1),timings[i]);
				strcpy(data_buffer,"1");
				send(new_socket,data_buffer,BUFFER_SIZE,0);	
				strcpy(data_buffer,"");
				sprintf(data_buffer,"%d : %s\n",i+1,timings[i]);
				send(new_socket,data_buffer,BUFFER_SIZE,0);
			}


			strcpy(data_buffer,"999");
			send(new_socket,data_buffer,BUFFER_SIZE,0);
			break;	

		}
	};
}

//SERVICE 2 : APPLICATION LAYER RTT MEASUREMENT
void AL_RTT(int new_socket,int client_id){

	int read_val;
	int iterations;
	int mssg_length=0;
	char data_buffer[BUFFER_SIZE] =""; 
	char mssg_buffer[BUFFER_SIZE]="";
	struct timeval start, stop;
	double secs = 0;
	double temp_secs = 0;

		//SEND THE MESSAGE TO ASK USER FOR MESSAGE LENGTH
		strcpy(data_buffer,"1");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		strcpy(data_buffer,"Enter Message Length:");
		send(new_socket,data_buffer,BUFFER_SIZE,0);

		//SCAN THE INPUT FROM USER FOR MESSAGE LENGTH
		strcpy(data_buffer,"2");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		strcpy(data_buffer,"");
		read_val = read( new_socket , data_buffer, BUFFER_SIZE ); 
		sscanf(data_buffer,"%d",&mssg_length);
		printf("\nClient %d : Message String Length:%d\n",client_id,mssg_length);

		//SEND THE MESSAGE TO ASK USER FOR NO. OF ITERATIONS
		strcpy(data_buffer,"1");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		strcpy(data_buffer,"Enter No.of Iterations:");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		
		//SCANNTHE INPUT FROM USER FOR ITERATIONS
		strcpy(data_buffer,"2");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		strcpy(data_buffer,"");
		read_val = read( new_socket , data_buffer, BUFFER_SIZE ); 
		printf("Client %d : Iterations:%s\n",client_id,data_buffer);
		sscanf(data_buffer,"%d",&iterations);

	//CREATE A DUMMY MESSAGE OF SIZE IN BYTES RECEIVED
	strcpy(mssg_buffer,"A");
	for(int i=1;i<mssg_length;i++)
		strcat(mssg_buffer,"A");
	
	//SEND THE MESSAGE FOR RECEIVED NO. OF ITERATIONS
	printf("Sending message of %d length for %d iterations...\n",mssg_length,iterations);
	for(int i=0;i<iterations;i++){

		temp_secs=0;

		//START THE TIMER
		gettimeofday(&start, NULL);

		//SEND-RECEIVE THE MESSAGE (1 RTT)
		strcpy(data_buffer,"3");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		send(new_socket,mssg_buffer,BUFFER_SIZE,0);
		strcpy(data_buffer,"");
		read_val = read( new_socket , data_buffer, BUFFER_SIZE ); 

		//STOP THE TIMER
		gettimeofday(&stop, NULL);

		//CALCULATE THE TIME IN  MILLISECONDS		
		temp_secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);

		//SUM THE TIME FOR EACH RTT 
		secs += temp_secs;
	}

	//SEND THE AVERAGE RTT OVER THE ITERATIONS
	strcpy(data_buffer,"1");
	send(new_socket,data_buffer,BUFFER_SIZE,0);	
	strcpy(data_buffer,"");
	sprintf(data_buffer,"RTT Time for %d iterations is : %f\n",iterations,secs/iterations);
	send(new_socket,data_buffer,BUFFER_SIZE,0);

	//BREAK THE CLIENT LOOP FOR SERVING THE SERVER FOR CURRENT SERVICE
	strcpy(data_buffer,"999");
	send(new_socket,data_buffer,BUFFER_SIZE,0);

}

//SERVICE 3 : TO UPLOAD A FILE AND GET INTO SERVER
void upload_file(int new_socket,int client_id){

	int read_val;
	char data_buffer[BUFFER_SIZE] =""; 
	struct timeval start, stop;
	double secs = 0;
	char file_buff[BUFFER_SIZE]="";
	char *filename;
	char filename1[FILENAME_SIZE];
	char filename2[FILENAME_SIZE];
	FILE *fp;

	//SEND THE MESSAGE TO ASK USER FOR MESSAGE LENGTH
		strcpy(data_buffer,"1");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		strcpy(data_buffer,"Enter Filename:");
		send(new_socket,data_buffer,BUFFER_SIZE,0);

		//SCAN THE INPUT FROM USER FOR MESSAGE LENGTH
		strcpy(data_buffer,"2");
		send(new_socket,data_buffer,BUFFER_SIZE,0);
		strcpy(data_buffer,"");
		read_val = read( new_socket , data_buffer, BUFFER_SIZE ); 
		strcpy(filename1,data_buffer);		
		printf("\nClient %d : Uploading File :%s\n",client_id,data_buffer);

	//START THE TIMER
	gettimeofday(&start, NULL);

	//EXTRACT FILENAME AND MAKE SERVER FILENAME ACCORDINGLY TO REPLICATE

	strcpy(filename2,filename1);
	filename = strtok(filename2,".");
	strcat(filename2,"_server.txt");
	printf("Client %d : Writing to File:%s\n",client_id,filename2);

	//OPEN FILE FOR WRITING
	fp=fopen(filename2,"w");	
	
	strcpy(data_buffer,"4");
	send(new_socket,data_buffer,BUFFER_SIZE,0);
	strcpy(data_buffer,filename1);
	send(new_socket,data_buffer,BUFFER_SIZE,0);

	strcpy(data_buffer,"6");
	send(new_socket,data_buffer,BUFFER_SIZE,0);


	//LOOP UNTIL END OF FILE		
	do{				
		strcpy(data_buffer,"");
		read_val = read( new_socket , data_buffer, BUFFER_SIZE ); 

		if(strcmp(data_buffer,"file not present")==0){
			printf("File not found in client...\n");
			break;
		}
		if(strcmp(data_buffer,"-999")!=0){
			fprintf(fp,"%s",data_buffer);
			//printf("%s\n",data_buffer);
		}
	}while(strcmp(data_buffer,"-999")!=0);				


	strcpy(data_buffer,"5");
	send(new_socket,data_buffer,BUFFER_SIZE,0);


	fclose(fp);

	//STOP THE TIMER : RECEIVED THE ENTIRE FILE
	gettimeofday(&stop, NULL);

	//CALCULATE THE TIME IN MILLISECONDS
	secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
	
	//SEND THE UPLOADING TIME TO CLIENT
	strcpy(data_buffer,"");
	sprintf(data_buffer,"%f",secs);


	strcpy(data_buffer,"1");
	send(new_socket,data_buffer,BUFFER_SIZE,0);
	sprintf(data_buffer,"Time Take to upload file is :%f",secs);
	send(new_socket,data_buffer,BUFFER_SIZE,0);

	//BREAK THE CLIENT LOOP FOR SERVING THE SERVER FOR CURRENT SERVICE
	strcpy(data_buffer,"999");
	send(new_socket,data_buffer,BUFFER_SIZE,0);
}

//EXTRA SERVICE TO SHOW DYNAMIC ADDING OF SERVICE WITH NO CHANGE IN CLIENT
void add_2_numbers(int new_socket,int client_id){
	int read_val;
	char data_buffer[BUFFER_SIZE] =""; 
	char mssg_buffer[BUFFER_SIZE]="";
	struct timeval start, stop;
	double secs = 0;
	double temp_secs = 0;
	int num1=0,num2=0;
	int sum=0;

	//SEND MESSAGE TO GET THE 1ST NUMBER AND SCAN IT FROM USER
	strcpy(data_buffer,"1");
	send(new_socket,data_buffer,BUFFER_SIZE,0);
	strcpy(data_buffer,"Enter first number:");
	send(new_socket,data_buffer,BUFFER_SIZE,0);

	strcpy(data_buffer,"2");
	send(new_socket,data_buffer,BUFFER_SIZE,0);
	strcpy(data_buffer,"");
	read_val = read( new_socket , data_buffer, BUFFER_SIZE ); 
	printf("Input 1:%s\n",data_buffer);
	sscanf(data_buffer,"%d",&num1);

	//SEND MESSAGE TO GET THE 2ND NUMBER AND SCAN IT FROM USER
	strcpy(data_buffer,"1");
	send(new_socket,data_buffer,BUFFER_SIZE,0);
	strcpy(data_buffer,"Enter second number:");
	send(new_socket,data_buffer,BUFFER_SIZE,0);

	strcpy(data_buffer,"2");
	send(new_socket,data_buffer,BUFFER_SIZE,0);
	strcpy(data_buffer,"");
	read_val = read( new_socket , data_buffer, BUFFER_SIZE ); 
	printf("Input 2:%s\n",data_buffer);
	sscanf(data_buffer,"%d",&num2);

	//SUM THE NUMBERS AND SEND IT TO CLIENT
	sum=num1+num2;
	strcpy(data_buffer,"1");
	send(new_socket,data_buffer,BUFFER_SIZE,0);
	sprintf(data_buffer,"Sum:%d\n",sum);
	send(new_socket,data_buffer,BUFFER_SIZE,0);

	//QUIT LOOP FOR SERVING THE CURRENT SERVICE
	strcpy(data_buffer,"999");
	send(new_socket,data_buffer,BUFFER_SIZE,0);

}

//CONTROLLER FUNCTION TO HANDLE EACH CLIENT (1 PER THREAD)
void *handle_client(int *new_socket1){

	char user_id[100];
	char password[100];
	int authenticated=-1;
	int new_socket;
	int read_val;	
	char buffer_choice[BUFFER_SIZE]=""; 
	char data_buffer[BUFFER_SIZE] =""; 
	struct timeval start, stop;
	double secs = 0;
	double temp_secs = 0;
	char services[BUFFER_SIZE];

	//INITIALIZE THE SERVICES
	initialize_services(services);
	new_socket=*new_socket1;		
	printf("\nServices Initialized for Client\n");	

	//RECEIVE THE CREDENTIALS (USER_ID AND PASSWORD)
	/*strcpy(data_buffer,"");
	read_val = read(new_socket,data_buffer,BUFFER_SIZE);
	strcpy(user_id,data_buffer);
	strcpy(data_buffer,"");
	read_val = read(new_socket,data_buffer,BUFFER_SIZE);
	strcpy(password,data_buffer);

	//CHECK FOR THE AUTHENTICATIONS
	for(int i=0;i<3;i++){
		if(strcmp(users[i].user_id,user_id)==0){
			if(strcmp(users[i].password,password)==0){
				authenticated=i;
			}
		}
	}	
	
	//RETURN IF AUTHENTICATION FAILS - ELSE SEND CLIENT CONFIRMATION
	if(authenticated==-1){
		send(new_socket,"not_auth",BUFFER_SIZE,0);
		printf("Client Authentication Failed\n",authenticated);
		return;
	}else
		send(new_socket,data_buffer,BUFFER_SIZE,0);
	printf("Client '%s' Authenticated : Client Id %d\n",users[authenticated].user_id,authenticated);
	strcpy(data_buffer,"");
	sprintf(data_buffer,"%d",authenticated);*/

	

 	
	//MANAGE THE CASE FOR SERVICE CHOSEN AS INPUT FROM USER
	while(1){
		//SEND THE SERVICES TO CLIENT
		printf("Socket Number:%d\n",new_socket);
		send(new_socket , services , BUFFER_SIZE , 0 );

		if(modified_flag==0){
		strcpy(data_buffer,"-----");
 		send(new_socket , data_buffer , BUFFER_SIZE ,0);
		}
		strcpy(buffer_choice,"");
		//strcpy(data_buffer,"---");
		//send(new_socket,data_buffer,);
		strcpy(data_buffer,"");

		read_val = read( new_socket , buffer_choice, BUFFER_SIZE);
		printf("\nService Selected by client %d is :  %s\n",authenticated,buffer_choice); 
		if(strcmp(buffer_choice,"1")==0){
			echo_request_reply(new_socket,authenticated);
		}		
		else if(strcmp(buffer_choice,"2")==0){
			AL_RTT(new_socket,authenticated);
		}	
		else if(strcmp(buffer_choice,"3")==0){
			upload_file(new_socket,authenticated);
		}
		else if(strcmp(buffer_choice,"4")==0){
			add_2_numbers(new_socket,authenticated);
		}
		else if(strcmp(buffer_choice,"999")==0){
			printf("Client %d Exited : Thread Closed\n\n",authenticated);
			break;
		}

	}

}


int main(int argc, char const *argv[]) 
{

	//INITIALIZE THE CONNECTION AND SOCKET ALONG WITH EXCEPTION HANDLING
	int server_fd, new_socket, read_val; 
	struct sockaddr_in address; 
	int opt = 1; 
	pthread_t tid,tid_1; 
	int addrlen = sizeof(address); 	
	modified_flag=0;
	printf("ARGC :%d .\n",argc);
	if(argc == 1)
	{
		printf("This is first server.\n");
		//printf("::::::::::::::::::%s,\n",argv[0]);
		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){ 
			perror("Socket Failed."); 
			exit(EXIT_FAILURE); 
		} 
		
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))){ 
			perror("Set Socket Option"); 
			exit(EXIT_FAILURE); 
		} 
		address.sin_family = AF_INET; 
		address.sin_addr.s_addr = INADDR_ANY; 
		address.sin_port = htons( PORT ); 
		
		//BIND THE ADDRESS AND PORT
		if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0){ 
			perror("Binding Failed"); 
			exit(EXIT_FAILURE); 
		} 
		if (listen(server_fd,NO_OF_USERS) < 0){ 
			perror("listen"); 
			exit(EXIT_FAILURE); 
		} 

		//INIT USER CREDENTIALS
		initialize_user_credentials(users);

		printf("\nServer Listening to requests from client...\n");
		
		pthread_create( &tid_1 , NULL , getFileCreationTime , NULL ); 
		//LISTEN TO USER IF ANY
		while(1){
			//getFileCreationTime();
			//CHECK FOR CONNECTION ACCEPT BY ANY CLIENT
			if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0)
			{ 
				perror("Accept Received"); 
				exit(EXIT_FAILURE); 
			} 			
			printf("%d\n",new_socket);
			//printf("%c\n",new_socket);
			//SERVE THE CLIENT IN A SEPERATE THREAD
			socketList[count] = new_socket;
			printf("%d\n",socketList[0]);
			count++;
			pthread_create(&tid, NULL, handle_client,(void *) &new_socket); 

		}
	}
	else
	{

		//CREATING THREAD FOR EXISTING CLIENTD
		char data_buffer[BUFFER_SIZE];
		int read_val;

		printf("Server Modified ...\n");
		for( int i = 1 ; i < argc ; i++ )
		{
			printf(">>>>>>>>>>>>>>>>>>>>>>",i,argv[i]);
			//printf("Server Updated\n",argv[i]);
			///printf("Count:%d\n",count);
			sscanf(argv[i],"%d",&socketList[count]);
			printf("Socket List:%d\n",socketList[count]);

	
			read_val = read(socketList[count],data_buffer, BUFFER_SIZE);		
			printf("Discarding Choice  :%s\n",data_buffer);		
			printf("Restarting services for client with socket:%d\n",socketList[count]);
			pthread_create(&tid, NULL, handle_client,(void *) &socketList[count]);
			count++;
		}

//#######################################################################################################################
		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){ 
			perror("Socket Failed."); 
			exit(EXIT_FAILURE); 
		} 
		
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))){ 
			perror("Set Socket Option"); 
			exit(EXIT_FAILURE); 
		} 
		address.sin_family = AF_INET; 
		address.sin_addr.s_addr = INADDR_ANY; 
		address.sin_port = htons( PORT ); 
		
		//BIND THE ADDRESS AND PORT
		if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0){ 
			perror("Binding Failed"); 
			exit(EXIT_FAILURE); 
		} 
		if (listen(server_fd,NO_OF_USERS) < 0){ 
			perror("listen"); 
			exit(EXIT_FAILURE); 
		} 


		//INIT USER CREDENTIALS
		initialize_user_credentials(users);

		//ACCEPTING NEW CLIENTS
		pthread_create( &tid_1 , NULL , getFileCreationTime , NULL );
		while(1)
		{
			//getFileCreationTime();
			//CHECK FOR CONNECTION ACCEPT BY ANY CLIENT
			if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0)
			{ 
				perror("Accept Received"); 
				exit(EXIT_FAILURE); 
			} 

			//SERVE THE CLIENT IN A SEPERATE THREAD
			socketList[count] = new_socket;
			count++;
			pthread_create(&tid, NULL, handle_client,(void *) &new_socket); 
		}
	}
	return 0; 
} 

