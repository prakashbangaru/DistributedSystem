/* 
 * Client.c 
 * usage: client.out <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <netdb.h> 
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include "Client.h"

int connectMaster();

char masterHostName[MASTER_HOST_NAME_LENGTH];
int masterPortNumber;

int main(int argc, char **argv) {
	int  portno =0;
	int  n =0;
	int sockfd=0;          
	struct hostent *server=NULL;
	struct hostent *clientIP=NULL;
	char *hostname=NULL;
	int32_t queryType=0;
	int32_t numberOfDistributionPoints=0;
	/*For USer- User Type is 1 and for client User Type is 2*/
	int32_t userType = CLIENT_USER_TYPE;
        int32_t messageType=0;
        int32_t clientBindPort = CLIENT_PORT;  
        struct sockaddr_in serveraddr;
        
       
	/* check command line arguments */
	if (argc != 3) {
		fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
		exit(0);
	}
	hostname = argv[1];
	portno = atoi(argv[2]);
        masterPortNumber = portno;
        memset(masterHostName,'\0',MASTER_HOST_NAME_LENGTH); 
        memcpy(masterHostName,argv[1],strlen(argv[1]));

	/* gethostbyname: get the server's DNS entry */
	server = gethostbyname(hostname);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host as %s\n", hostname);
		exit(0);
	}
	/* build the server's Internet address */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
			(char *)&serveraddr.sin_addr.s_addr, server->h_length);
	serveraddr.sin_port = htons(portno);
        

	QueryDetailsType queryDetails;

	memset(&queryDetails,'\0',sizeof(queryDetails));

	/* socket: create the socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	/* connect: create a connection with the server */
	n = connect(sockfd, &serveraddr, sizeof(serveraddr));
	if(n < 0){
		error("ERROR connecting");
	}

	/*Send the USER type to the Server*/
	n= write(sockfd,&userType, sizeof(userType)); 
	if (n < 0){ 
		error("ERROR writing User Type  to the socket in Client");
	}

	/*Send the MESSAGE type to the Server*/
        messageType = CLIENT_IDENTITY_MESSAGE_TYPE;
	n= write(sockfd,&messageType, sizeof(messageType)); 
	if (n < 0){ 
		error("ERROR writing Message Type  to the socket in Client");
	}

	/*Send Client Identity to the Server here, We send only Port number. 
         Server Will get the IP address of the Client from socket details
         This is because client may not aware of its public ip or 
         Difficult ot get the clients pulic IP in the client Side*/
	n= write(sockfd,&clientBindPort, sizeof(clientBindPort)); 
	if (n < 0){ 
		error("ERROR writing Client Identity  to the socket in Client");
	}
        close(sockfd); 
        /*Spawn Thread for client to listen to the Servere for the Query to be received*/
        pthread_t thread_id;
        if( pthread_create( &thread_id , NULL ,  clientListeningFunction ,NULL) < 0){
	           perror("could not create thread");
	           return 1;
        }
        pthread_join( thread_id , NULL);
        return 0;
}

void error(char *msg) {
	perror(msg);
	exit(0);
}


int connectMaster(){
        int n =0;
	int sockfd=0;
        struct hostent *server=NULL;
        char *hostname=NULL;
        struct sockaddr_in serveraddr;

        printf("Master Host Name is : %s\n",masterHostName);
        /* gethostbyname: get the server's DNS entry */
        server = gethostbyname(masterHostName);
        if (server == NULL) {
                fprintf(stderr,"ERROR, no such host as %s\n", hostname);
                exit(0);
        }

        /* build the server's Internet address */
        bzero((char *) &serveraddr, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
                        (char *)&serveraddr.sin_addr.s_addr, server->h_length);
        serveraddr.sin_port = htons(masterPortNumber);
        
        /* socket: create the socket */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                error("ERROR opening socket");

        /* connect: create a connection with the server */
        n = connect(sockfd, &serveraddr, sizeof(serveraddr));
        if(n < 0){
                error("ERROR connecting");
        }

        return sockfd;
}

int sendFile(int sockfd, char *fileName){
	int fp=-1;
	struct stat file_stat;
	int n =0;
	long offset =0;
	long remain_data =0;
	long sent_bytes=0;
	long file_size=0;

	fp = open(fileName, O_RDONLY);
	if(fp == -1){
		error("ERROR in Opening the File");
	}

	/* Get file stats */
	if (fstat(fp, &file_stat) < 0){
		fprintf(stderr, "Error fstat --> %s", strerror(errno));
		exit(EXIT_FAILURE);
	} 

	fprintf(stdout, "File Size: %d bytes\n", file_stat.st_size);
	file_size=file_stat.st_size;
	/* Sending file size */
	n = write(sockfd, &file_size , sizeof(file_size));
	if (n < 0){
		fprintf(stderr, "Error on sending File Size %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
	offset = 0;
	remain_data = file_stat.st_size;

	/* Sending file data */
	while (((sent_bytes = sendfile(sockfd, fp, &offset, BUFSIZ)) > 0) && (remain_data > 0))
	{
		fprintf(stdout, "1. Server sent %ld bytes from file's data, offset is now : %ld and remaining data = %ld\n", sent_bytes, offset, remain_data);
		remain_data -= sent_bytes;
		fprintf(stdout, "2. Server sent %ld bytes from file's data, offset is now : %ld and remaining data = %ld\n", sent_bytes, offset, remain_data);
	}

}

int sendData(int sock, char *message){
	int32_t messageLength = strlen(message);
	int n = 0;
	/*Send MEssage Length*/
	n = write(sock, &messageLength, sizeof(int32_t));
	if(n<0){
		error("ERROR writing messageLength");
	}
	/*Send Messsage*/
	n = write(sock, message, messageLength);
	return n;
}

void* clientListeningFunction(void *arg){
	int client_desc , server_desc , c;
	struct sockaddr_in server , client;
	//Create socket
	client_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (client_desc == -1)
	{
		printf("Could not create socket");
	}
	printf("Socket created");
	//Prepare the sockaddr_in structure
	client.sin_family = AF_INET;
	client.sin_addr.s_addr = INADDR_ANY;
	client.sin_port = htons(CLIENT_PORT);
	//Bind
	if( bind(client_desc,(struct sockaddr *)&client , sizeof(client)) < 0)
	{
		//print the error message
		perror("bind failed at Client side. Error");
		exit(EXIT_FAILURE);
	}
	printf("bind done\n");
	//Listen
	listen(client_desc , 1);
	//Accept incoming connection
	printf("Waiting for  Query Data from Server..\n");
	c = sizeof(struct sockaddr_in);

	while( (server_desc = accept(client_desc, (struct sockaddr *)&server, (socklen_t*)&c)) )
	{
		printf("Connection accepted\n");
		connectionHandler(server_desc);
	}
}

void connectionHandler(int sock){
	int readSize=0;
	int32_t queryType=0;
	double piValue=0.0;
        int32_t messageType =0;

	if( (readSize = recv(sock , &messageType , sizeof(messageType) , 0)) > 0 ){
		if(messageType == FILE_SORT_MESSAGE_TYPE){
                        printf("Received Message Type of File SortQuery from  Master\n");
			fileHandler(sock);
		}
                else{
			if(messageType == MONTECARLO_MESSAGE_TYPE){
			   monteCarloHandler(sock);	
                        }
                }
        }
}

void monteCarloHandler(int sock){
     int userType = CLIENT_USER_TYPE;
     int messageType = MONTECARLO_MESSAGE_TYPE;
     int socketMaster=0;
     int readSize =0;
     int n=0;
     UserMonteCarloQueryTable userMonteCarloQuery;

     /*Receive User Montecarlo Query from Master*/
     if( (readSize = recv(sock , &userMonteCarloQuery , sizeof(UserMonteCarloQueryTable) , 0)) > 0 ){
               close(sock);

          /*Compute MonteCarlo Circular Points */
           processMonteCarlo(userMonteCarloQuery.clientsDistributionCount, &userMonteCarloQuery.totalCircularCount);

          /*connect to the Master*/
          socketMaster = connectMaster();

          /*Send the USER type to the Master*/
          n= write(socketMaster,&userType, sizeof(userType));
          if (n < 0){
                 error("ERROR writing User Type  to the Master in Client");
          }

          /*Send the MESSAGE type to the Master*/
          n= write(socketMaster,&messageType, sizeof(messageType));
          if (n < 0){
                  error("ERROR writing Message Type  to the Master in Client");
          }

          /*Send UserFile Query*/
          n = write(socketMaster,(void*) &userMonteCarloQuery, sizeof(UserMonteCarloQueryTable));
          if (n < 0){
                  fprintf(stderr, "Error on sending userMonteCarloQuery to Master\n");
                  exit(EXIT_FAILURE);
           }

          
          //close(socketMaster);                          
     }
}

void processMonteCarlo(int32_t num_points, int *num_circle)
{
    struct drand48_data drandBuf;
    int i=0;
    double x_val= 0.0;
    double y_val=0.0;
    double point_dist=0.0;
    *num_circle=0;
    for (i=0; i<num_points; i++){
          drand48_r(&drandBuf, &x_val);
          drand48_r(&drandBuf, &y_val);
          point_dist = (x_val*x_val) + (y_val*y_val);
 	  if (point_dist<=1)
               *num_circle += 1;
    }
   printf("At Client Total points = %d \t num_circle = %d\n", num_points, *num_circle);
}
             

void fileHandler(int sock){
    int readSize=0;
    FILE *fp=NULL;
    int socketMaster=0;
    int n=0;
    int32_t userType = CLIENT_USER_TYPE;
    int32_t messageType = FILE_SORT_MESSAGE_TYPE;
    int32_t sortedFileSize =0;
    char* sortedFileContent=NULL;
    int32_t fileSize=0;

    UserFileQueryTable userFileQuery;

    memset(&userFileQuery,'\0',sizeof(UserFileQueryTable));

	/*Receive User file Query from Master*/
	if( (readSize = recv(sock , &userFileQuery , sizeof(UserFileQueryTable) , 0)) > 0 ){

               printf("Received File SortQuery from  Master, File Size is %d\n",userFileQuery.fileSize);
               fp = fopen(userFileQuery.fileName, "w");
                if(fp == NULL){
                        error("file OPen Failed");
                }
        	userFileQuery.fileContent = malloc(sizeof(char)*userFileQuery.fileSize);
                if(userFileQuery.fileContent == NULL){
                    error("Memory Allocation Failed for File Content to be received from the Master\n");
                }
		memset(userFileQuery.fileContent,'\0',sizeof(char)*userFileQuery.fileSize);

                /*Receive File Content*/
		if( (readSize = recv(sock , userFileQuery.fileContent , sizeof(char)*(userFileQuery.fileSize) , 0)) > 0 ){
                        printf("File content: %s \n Received Successfully in the client\n",userFileQuery.fileContent);
                	close(sock);
                        /*Write the File Content into a File*/
	                fwrite(userFileQuery.fileContent, sizeof(char), readSize, fp);
                        free(userFileQuery.fileContent);
			fclose(fp);
 
                        /*Sort the File Content*/
                        sortedFileContent = sortFileContent(userFileQuery.fileName,&sortedFileSize);
                        //printf("Sorted File Content = %s\n",sortedFileContent);
                         sortedFileSize = strlen(sortedFileContent);
                        /*Update the User File Query with the Sorted File Content*/
                        userFileQuery.fileContent = sortedFileContent;

                        /*UPdate the File Size after Sorting*/
                        userFileQuery.fileSize = sortedFileSize;

                        //BVP Do File Sorting
                        socketMaster = connectMaster();

                        /*Send the USER type to the Master*/
		        n= write(socketMaster,&userType, sizeof(userType));
        		if (n < 0){
                		error("ERROR writing User Type  to the socket in Client");
        		}
                        
                        /*Send the MESSAGE type to the Master*/
		        messageType = FILE_SORT_MESSAGE_TYPE;
        		n= write(socketMaster,&messageType, sizeof(messageType));
        		if (n < 0){
                		error("ERROR writing Message Type  to the socket in Client");
        		}

                        /*Send UserFile Query*/
		        n = write(socketMaster,(void*) &userFileQuery, sizeof(UserFileQueryTable));
           		if (n < 0){
               			 fprintf(stderr, "Error on sending userFileQuery to Master\n");
               			 exit(EXIT_FAILURE);
            		}

                        /*Send the File Content to the Master*/
           		 n = write(socketMaster,(void*)userFileQuery.fileContent, sizeof(char)*userFileQuery.fileSize);
           		 if (n < 0){
               			 fprintf(stderr, "Error on sending file Content to the Master\n");
               			 exit(EXIT_FAILURE);
            		 }
                         close(socketMaster);
 		}
 	}
}

int receiveFile(int sock){
	FILE *fp=NULL;
	int readSize=0;
	long file_size=0;
	long remain_data = 0;
	char buffer[BUFSIZ];
	int len=0;

	memset(buffer,'\0',BUFSIZ);
	// Receive File Name
	readSize = receiveData(sock , buffer);
	if( readSize <= 0 ){
		error("Fail to receive the File Name\n");
	}
	else{
		printf("fileName is %s\r\n",buffer);
		fp = fopen("SampleClient.txt", "w");
		if(fp == NULL){
			error("file OPen Failed");
		}
		/* Receiving file size */
		recv(sock,&file_size,sizeof(file_size),0);
		remain_data = file_size;
		printf("Received file Size is at Client side :%ld\r\n",file_size);

		while (((len = recv(sock, buffer, BUFSIZ, 0)) > 0) && (remain_data > 0))
		{
			fwrite(buffer, sizeof(char), len, fp);
			remain_data -= len;
			fprintf(stdout, "Receive %d bytes and we hope :- %ld bytes\n", len, remain_data);
		}
		fclose(fp);
	}

}

int receiveData(int sock,char *message){
	int32_t messageLength=0;
	int readSize=0;

	if( (readSize = recv(sock , &messageLength , sizeof(messageLength) , 0)) > 0 ){
		printf("Message Length : %d\n",messageLength);
		readSize = recv(sock ,message, messageLength, 0);
		printf("Message : %s\n",message);
	}
	return readSize;
}

