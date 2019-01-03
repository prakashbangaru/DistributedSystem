#include <stdio.h>
#include <string.h>    
#include <stdlib.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>   
#include <pthread.h> 
//#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include "Master.h"

ClientIdentityType clientDetails[MAX_CLIENT];
int clientIdentityIndex=0; 
pthread_mutex_t CLIENT_IDENTITY_MUTEX;

void clientFileHandler(int clientSocket);

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c;
	struct sockaddr_in server , client;

        if(pthread_mutex_init(&CLIENT_IDENTITY_MUTEX, NULL) != 0) 
        { 
            printf("\n mutex init has failed for CLIENT_IDENTITY_MUTEX\n"); 
            exit(EXIT_FAILURE); 
        } 
     
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(SERVER_PORT);

	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	//Listen
	listen(socket_desc , 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	pthread_t thread_id;

	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
                struct in_addr ipAddr = client.sin_addr;
                char ipAddressStr[INET_ADDRSTRLEN];
                ThreadAttribute threadAttr;

                memset(ipAddressStr,'\0',INET_ADDRSTRLEN);
                memset(&threadAttr,'\0',sizeof(threadAttr));
                inet_ntop( AF_INET, &ipAddr, threadAttr.ipAddress, INET_ADDRSTRLEN );
                
                threadAttr.sock = client_sock;

		printf("Connection accepted from IP %s\r\n",threadAttr.ipAddress);
                //BVP Remove Below
		//connection_handler((void*) &threadAttr);
		if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &threadAttr) < 0)
		{
			perror("could not create thread");
			return 1;
		}
		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( thread_id , NULL);
	}

	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}

	return 0;
}

/*
 * This will handle connection for each Client/User
 * */
void *connection_handler(void *threadAttrParam)
{
	//Get the socket descriptor
	ThreadAttribute *threadAttr = (ThreadAttribute*)threadAttrParam;
	int read_size;
	int32_t userType=0;
        QueryDetailsType queryDetails;

        memset(&queryDetails,'\0',sizeof(queryDetails));    

        /*Get User Type*/
	if( (read_size = recv(threadAttr->sock , &userType , sizeof(userType) , 0)) > 0 ){
             if(userType == USER_USER_TYPE){
                 printf("Connect is of Type User\n");
                 userHandler(&queryDetails,threadAttr->sock,threadAttr->ipAddress);
             }
             else if(userType == CLIENT_USER_TYPE){
                 clientHandler(threadAttr);
             }
             else{
                  printf("Invalid UserType System Terminates\n");
                  exit(EXIT_FAILURE);
             }
        }
}

void clientHandler(ThreadAttribute *threadAttr){
       int32_t messageType=0;
       int readSize=0;
       int clientSocket = threadAttr->sock;

        if( (readSize = recv(clientSocket,&messageType,sizeof(messageType) , 0)) > 0 ){
              if(messageType == CLIENT_IDENTITY_MESSAGE_TYPE){
                    addClient(clientSocket,threadAttr->ipAddress);
              }
              if(messageType == FILE_SORT_MESSAGE_TYPE){
                   clientFileHandler(clientSocket);
              }
        }
}

void clientFileHandler(int clientSocket){

     UserFileQueryTable userFileQuery;
     FILE *fp=NULL;
     int userSocket =0;
     int32_t messageType= FILE_SORT_MESSAGE_TYPE;
     int n=0;
     int readSize=0;

     memset(&userFileQuery,'\0',sizeof(UserFileQueryTable));
    
        /*Receive User file Query from Client*/
        if( (readSize = recv(clientSocket , &userFileQuery , sizeof(UserFileQueryTable) , 0)) > 0 ){
                printf("Received File SortQuery Solution from  Client, File Size is %d\n",userFileQuery.fileSize); 

                userFileQuery.fileContent = malloc(sizeof(char)*userFileQuery.fileSize);
                if(userFileQuery.fileContent == NULL){
                    error("Memory Allocation Failed for File Content to be received from Client\n");
                }
                memset(userFileQuery.fileContent,'\0',sizeof(char)*userFileQuery.fileSize);

                /*Receive File Content*/
                if( (readSize = recv(clientSocket , userFileQuery.fileContent , sizeof(char)*(userFileQuery.fileSize) , 0)) > 0 ){
                        printf("File content: %s \n Received Successfully From the  the client\n",userFileQuery.fileContent);
                        close(clientSocket);
                        /*Write the File Content into a File*/
                	fp = fopen(userFileQuery.fileName, "w");
                	if(fp == NULL){
                        	error("file OPen Failed");
                	}
                        fwrite(userFileQuery.fileContent, sizeof(char), readSize, fp);
                        fclose(fp);
                }

                userSocket = connectUser(userFileQuery.userIpAddress,userFileQuery.userPortNumber);

                /*Send the MESSAGE type to the User*/
                n= write(userSocket,&messageType, sizeof(messageType));
                if (n < 0){
                    error("ERROR writing Message Type  to  User in Master \n");
                }

                 /*Send UserFile Query to User*/
                 n = write(userSocket,(void*) &userFileQuery, sizeof(UserFileQueryTable));
                 if (n < 0){
                      	fprintf(stderr, "Error on sending userFileQuery to USer in Master\n");
                        exit(EXIT_FAILURE);
                  }

                  /*Send the File Content to the User*/
                  n = write(userSocket,(void*)userFileQuery.fileContent, sizeof(char)*userFileQuery.fileSize);
                  if (n < 0){
                          fprintf(stderr, "Error on sending file Content to User in  Master\n");
                          exit(EXIT_FAILURE);
                   }
                  close(userSocket);
        }
}


int connectUser(char *userIpAddress, int userPortnumber){
 struct hostent *server=NULL;
 struct sockaddr_in serveraddr;
 int sockfd =0;
 int n=0;

      /* gethostbyname: get the server's DNS entry */
        server = gethostbyname(userIpAddress);
        if (server == NULL) {
                fprintf(stderr,"ERROR, no such host as %s\n",userIpAddress);
                exit(0);
        }

        /* build the server's Internet address */
        bzero((char *) &serveraddr, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
                        (char *)&serveraddr.sin_addr.s_addr, server->h_length);
        serveraddr.sin_port = htons(userPortnumber);

          /* socket: create the socket */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                error("ERROR opening socket while connecting to User\n");

        /* connect: create a connection with the USer */
        n = connect(sockfd, &serveraddr, sizeof(serveraddr));
        if(n < 0){
                error("ERROR connecting User\n");
        }
        return sockfd;
}

void addClient(int clientSocket,char *clientIpAddress){
        int readSize=0; 
        int32_t portNumber=0;
	/*Get the Port Number where Client is Listening*/
        if( (readSize = recv(clientSocket,&portNumber,sizeof(portNumber) , 0)) > 0 ){
            if(!isClientExists(clientIpAddress,portNumber)){
            	if(clientIdentityIndex < MAX_CLIENT){
	     		pthread_mutex_lock(&CLIENT_IDENTITY_MUTEX);
                	memcpy(clientDetails[clientIdentityIndex].ipAddress,clientIpAddress,INET_ADDRSTRLEN);
                	clientDetails[clientIdentityIndex].portNumber = portNumber;    
	        	clientIdentityIndex++;
                         printf("New Client Had Joined, Number of Available Client is %d\n",clientIdentityIndex); 
                        //BVP
                        // Here Call the function to update the Database table client info
	     		pthread_mutex_unlock(&CLIENT_IDENTITY_MUTEX); 
            	}
                else{
                    printf("Maximum Number of Clients Reached\r\n");
                }
           }
           else{
           	printf("The Newly Joined Client Already Exists\n");
           }
	}
}


void removeClient(char * clientIpAddress,int32_t portNumber){
     int index=0;
     for(index =0 ; index < clientIdentityIndex;index++){
     	if(clientDetails[index].portNumber == portNumber){
             if(0 ==  memcmp(clientDetails[index].ipAddress,clientIpAddress,INET_ADDRSTRLEN)){
	     		pthread_mutex_lock(&CLIENT_IDENTITY_MUTEX);
                        //BVP
                        // Here Call the function to update the Database table client info
                       updateClientDetails(index);
	     	       pthread_mutex_unlock(&CLIENT_IDENTITY_MUTEX); 
                  
             }
        }
     }
}

void updateClientDetails(int clientDetailIndex){
     int index = clientDetailIndex;
     for(index =clientDetailIndex ; index < clientIdentityIndex-1;index++){
		clientDetails[index].portNumber = clientDetails[index+1].portNumber;
    		memcpy(clientDetails[index].ipAddress,clientDetails[index+1].ipAddress,INET_ADDRSTRLEN);
      }
      clientIdentityIndex--; 
}


bool isClientExists(char *clientIpAddress,int32_t portNumber){
     int index=0;
     for(index =0 ; index < clientIdentityIndex;index++){
     	if(clientDetails[index].portNumber == portNumber){
        	if(0 ==  memcmp(clientDetails[index].ipAddress,clientIpAddress,INET_ADDRSTRLEN)){
        		return true;
	     	}
	}
      }
      return false;
}

void userHandler(QueryDetailsType *queryDetails,int sock,char *ipAddress){
        int read_size =-1;
        printf("Waiting to Receive Query Details\n"); 
	if( (read_size = recv(sock , queryDetails , sizeof(QueryDetailsType) , 0)) > 0 ){
           memcpy(queryDetails->ipAddress,ipAddress,INET_ADDRSTRLEN);

           if(queryDetails->queryType == FILE_SORT_MESSAGE_TYPE){
              printf("Received File Sort Query from IP =%s port Number =%d Query id=%d FileSize = %d fileName = %s\n",\
                      queryDetails->ipAddress,queryDetails->portNumber,queryDetails->queryId,queryDetails->fileSize,queryDetails->queryDetail);
              userFileHandler(queryDetails, sock);
           }
           else if(queryDetails->queryType == MONTECARLO_MESSAGE_TYPE){
              printf("Received MonteCarlo  Query from IP =%s port Number =%d  Query id=%d\n",\
                      queryDetails->ipAddress,queryDetails->portNumber,queryDetails->queryId);
              userMontecarloHandler(queryDetails, sock);
           }
        }
}

void userMontecarloHandler(QueryDetailsType *queryDetails, int sock){
     int distributionCount=0;         
     printf("Distribution Count from USER is : %s\n",queryDetails->queryDetail);
     distributionCount = atoi(queryDetails->queryDetail);
    
}

void userFileHandler(QueryDetailsType *queryDetails, int sock){
    
     char fileNameServerSide[FILE_NAME_LENGTH];
     int totalFileSize =0;
     int numberOfClient=0;
     int masterHandleFileSize=0;
     int slaveHandleFileSize=0;
     int remainingFileSize =0;
     int equalFileSize =0;
     char *fileContent=NULL;
     int startIndex=0;
     int endIndex=0;
     int32_t numberOfLines =0;
     int32_t index=0;
     int clientSocket =0;
     int n =0;
     int32_t messageType = FILE_SORT_MESSAGE_TYPE; 
     char underScore='_';
     UserFileQueryTable userFileQuery;
     FILE *masterunSortedfp=NULL;

     memset(&userFileQuery,'\0',sizeof(UserFileQueryTable));
 
     memset(fileNameServerSide,'\0',FILE_NAME_LENGTH);
     memcpy(fileNameServerSide,queryDetails->queryDetail,strlen(queryDetails->queryDetail));

     receiveFileFromUser(sock,queryDetails);
     close(sock);
     totalFileSize = queryDetails->fileSize;
     fileContent =  queryDetails->fileContent;    

      pthread_mutex_lock(&CLIENT_IDENTITY_MUTEX);
      numberOfClient = clientIdentityIndex+1; /*Including the Master*/
      pthread_mutex_unlock(&CLIENT_IDENTITY_MUTEX);

     getUserFileQuery(&userFileQuery,queryDetails,numberOfClient);
     /*BVP Update the File conetent and File Query into DataBase*/ 

      ClientFileQueryMappingTable clientFileQueryMapping[numberOfClient];

      memset(clientFileQueryMapping,'\0',sizeof(ClientFileQueryMappingTable)*numberOfClient);

      remainingFileSize = totalFileSize % numberOfClient;
      equalFileSize = totalFileSize / numberOfClient;
      masterHandleFileSize = equalFileSize + remainingFileSize;
      slaveHandleFileSize = equalFileSize;
 
      startIndex =0;
      endIndex= getEndIndex(masterHandleFileSize,fileContent);

      clientFileQueryMapping[0].startIndex = startIndex; 
      clientFileQueryMapping[0].endIndex = endIndex;
      

      memcpy(clientFileQueryMapping[index].userIpAddress,queryDetails->ipAddress,INET_ADDRSTRLEN); 
      clientFileQueryMapping[index].userPortNumber = queryDetails->portNumber; 
      
      clientFileQueryMapping[index].queryId = queryDetails->queryId;

      sprintf(clientFileQueryMapping[index].fileName,"%s%c%d",queryDetails->queryDetail,underScore,index); 
      strcpy(clientFileQueryMapping[0].clientIpAddress,"0.0.0.0");
      clientFileQueryMapping[0].clientPortNumber = 0;

      /*Write Master Part Unsorted Chunk File*/ 
      masterunSortedfp = fopen(clientFileQueryMapping[index].fileName, "w");
      if(masterunSortedfp == NULL){
                        error("file OPen Failed");
      }
      fwrite(fileContent, sizeof(char), endIndex, masterunSortedfp);
      fclose(masterunSortedfp);
      
      /*BVP Remove*/
      printf("Number Of Availbale Client is %d\n",numberOfClient);
      printf("Master Start IndexLine Number %d\n",clientFileQueryMapping[0].startIndex); 
      printf("Master End IndexLine Number %d\n",clientFileQueryMapping[0].endIndex); 
       
      /*update Client Mapping Table Structure*/    
      for(index =1 ;index < numberOfClient ; index++){
            if(endIndex >= totalFileSize ){
               break;
            }

	    startIndex = endIndex+1;
            endIndex = getEndIndex((endIndex + slaveHandleFileSize), fileContent);

            memcpy(clientFileQueryMapping[index].userIpAddress,queryDetails->ipAddress,INET_ADDRSTRLEN); 
            clientFileQueryMapping[index].userPortNumber = queryDetails->portNumber; 

            clientFileQueryMapping[index].queryId = queryDetails->queryId;

            sprintf(clientFileQueryMapping[index].fileName,"%s%c%d",queryDetails->queryDetail,underScore,index);            
            
            memcpy(clientFileQueryMapping[index].clientIpAddress,clientDetails[index-1].ipAddress,INET_ADDRSTRLEN);
            clientFileQueryMapping[index].clientPortNumber = clientDetails[index-1].portNumber; 
	    
            clientFileQueryMapping[index].startIndex=startIndex; 
	    clientFileQueryMapping[index].endIndex= endIndex;
            /*BVP Remove*/
            printf("Slave Start IndexLine Number %d\n",startIndex); 
            printf("Slave End IndexLine Number %d\n",endIndex);
      }

      /*BVP try to do this in Parallel Programming*/
      /* Connect to the Client and Send the chunk file to sort*/
      for(index=1;index < numberOfClient;index++){
           if(clientFileQueryMapping[index].startIndex <=0){
                 break;
            }
            clientSocket = connectToClient(clientFileQueryMapping[index].clientIpAddress,clientFileQueryMapping[index].clientPortNumber);
            startIndex = clientFileQueryMapping[index].startIndex;
            endIndex = clientFileQueryMapping[index].endIndex;
            /*UPdate the Chunk File Size to be sent to the client*/
            userFileQuery.fileSize = (endIndex - startIndex) + 1;

            /*Update The Chunk File Name for Each Client*/ 
            memset(userFileQuery.fileName,'\0',strlen(userFileQuery.fileName));   
            sprintf(userFileQuery.fileName,"%s%c%d",queryDetails->queryDetail,underScore,index);            

            /*Send Message Type*/
            n =  write(clientSocket,(void*) &messageType, sizeof(messageType));
            if (n < 0){
                fprintf(stderr, "Error on sending message Type to the Client  %s,%s\n",clientFileQueryMapping[index].clientIpAddress,strerror(errno));
                exit(EXIT_FAILURE);
             }


            /*Send User File Query*/
            n = write(clientSocket,(void*) &userFileQuery, sizeof(UserFileQueryTable));
            if (n < 0){
                fprintf(stderr, "Error on sending userFileQuery to the Client  %s,%s\n",clientFileQueryMapping[index].clientIpAddress,strerror(errno));
                exit(EXIT_FAILURE);
             }
            
            /*Send the File Content to the Client*/
            n = write(clientSocket,(void*) &fileContent[startIndex], sizeof(char)*userFileQuery.fileSize);
            if (n < 0){
                fprintf(stderr, "Error on sending file Content to the Client  %s,%s\n",clientFileQueryMapping[index].clientIpAddress,strerror(errno));
                exit(EXIT_FAILURE);
             }
             printf("File Content sent to the Client is : %s\n",&fileContent[startIndex]);
             
      }

       /*Do sort Opreation For the Master Data Part*/ 
       index =0;
       int userSocket=0;
       //char work[5]={'A','B','C','D','\0'};
        char *sortedFileContent=NULL;
       int32_t sortedFileSize=0;
       FILE *masterSortedfp=NULL;
   
       startIndex = clientFileQueryMapping[index].startIndex;
       endIndex = clientFileQueryMapping[index].endIndex;

       /*Update the Chunk File Size to be handled by the Master and to be  sent to the User*/
        userFileQuery.fileSize = (endIndex - startIndex) + 1;
                 
        /*Update the Chunk File Name for Master*/ 
        memset(userFileQuery.fileName,'\0',strlen(userFileQuery.fileName));   
        sprintf(userFileQuery.fileName,"%s%c%d",queryDetails->queryDetail,underScore,index);  
        
        /* BVP Sort the File chunk of the Master*/          
        sortedFileContent = sortFileContent(userFileQuery.fileName,&sortedFileSize);
        userFileQuery.fileSize = strlen(sortedFileContent);
        masterSortedfp = fopen(userFileQuery.fileName, "w");
        if(masterSortedfp == NULL){
                        error("file OPen Failed");
         }
        fwrite(sortedFileContent, sizeof(char), userFileQuery.fileSize, masterSortedfp);
        fclose(masterSortedfp);

        /*Send the File Sort MAster Chunk Data*/
        userSocket = connectUser(userFileQuery.userIpAddress,userFileQuery.userPortNumber);
       
        /*Send Message Type*/
        n =  write(userSocket,(void*) &messageType, sizeof(messageType));
        if (n < 0){
                fprintf(stderr, "Error on sending message Type to the USer for the Master File Chunk Data  %s,%s\n",clientFileQueryMapping[index].clientIpAddress,strerror(errno));
                exit(EXIT_FAILURE);
         }
         /*Send User File Query*/
         n = write(userSocket,(void*) &userFileQuery, sizeof(UserFileQueryTable));
         if (n < 0){
             fprintf(stderr, "Error on sending userFileQuery to the USer for the Master File Chunk Data  %s,%s\n",clientFileQueryMapping[index].clientIpAddress,strerror(errno));
             exit(EXIT_FAILURE);
          }
         //n = write(userSocket,(void*)work, sizeof(char)*5);
         n = write(userSocket,(void*) sortedFileContent, sizeof(char)*userFileQuery.fileSize);
         if (n < 0){
             fprintf(stderr, "Error on sending file Content to the USer for the Master File Chunk Data  %s,%s\n",clientFileQueryMapping[index].clientIpAddress,strerror(errno));
             exit(EXIT_FAILURE);
         }
         printf("Master Chunk Sorted File Content sent to the USer is : %s\n",sortedFileContent);
         free(sortedFileContent);
         //sleep(60);
         //close(userSocket);
 
     if(fileContent!=NULL)
        free(fileContent);
     
}

int connectToClient(char *hostName,int32_t clientPortNumber){

         struct hostent *server=NULL;
         int sockfd=0;
         struct sockaddr_in serveraddr; 
         int n = -1;

        /* gethostbyname: get the server's DNS entry */
        server = gethostbyname(hostName);
        if (server == NULL) {
                fprintf(stderr,"ERROR, no such host as %s\n", hostName);
                exit(0);
        }

        /* build the server's Internet address */
        bzero((char *) &serveraddr, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,(char *)&serveraddr.sin_addr.s_addr, server->h_length);
        serveraddr.sin_port = htons(clientPortNumber);

        /* socket: create the socket */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        	error("ERROR opening socket");

        /* connect: create a connection with the server */
        printf("connecting to the Client %s,%d\n",hostName,clientPortNumber);
        n = connect(sockfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr));
        if(n < 0){
            	error("ERROR connecting to Client");
         }
         printf("connected to the Client %s,%d\n",hostName,clientPortNumber);
            
         return sockfd;
}

int getEndIndex(int fileDataIndex,char *fileContent){
int forwardIndex= fileDataIndex;
int backwardIndex = fileDataIndex;
        while(1){

	 	if(fileContent[backwardIndex]!='\n'&& fileContent[backwardIndex]!='\0'){
        	   	backwardIndex--;
                 }
                else{
                       return backwardIndex;
                } 

	 	if(fileContent[forwardIndex]!='\n'&& fileContent[forwardIndex]!='\0'){
        	   	forwardIndex++;
                }
                else{
                        return forwardIndex;
                }
 	}
 	//return fileDataIndex++;
}

void getUserFileQuery(UserFileQueryTable *userFileQuery,QueryDetailsType *queryDetails,int numberOfClients){
            memcpy(userFileQuery->userIpAddress,queryDetails->ipAddress,INET_ADDRSTRLEN); 
            userFileQuery->userPortNumber = queryDetails->portNumber; 
            userFileQuery->queryId = queryDetails->queryId;
	    userFileQuery->fileContent = queryDetails->fileContent;
            userFileQuery->numberOfClients = numberOfClients;
            userFileQuery->fileSize = queryDetails->fileSize;
            //userFileQuery->fileLineCount = fileLineCount;
}

int32_t receiveFileFromUser(int sock,QueryDetailsType *queryDetails){
   FILE *fp=NULL;
   int read_size=0;
   int32_t remain_data = 0;
   char buffer[BUFSIZ];
   int32_t len=0;
   int32_t fileLineCount=0;
   int32_t index=0;

        memset(buffer,'\0',BUFSIZ);

		fp = fopen(queryDetails->queryDetail, "w");
		if(fp == NULL){
			error("file OPen Failed");
		}
            
                queryDetails->fileContent = (char*)malloc(sizeof(char)*(queryDetails->fileSize));
                
		if(queryDetails->fileContent!=NULL){
                   memset(queryDetails->fileContent,'\0',sizeof(char)*queryDetails->fileSize);
                }
                else{
			error("File Content Memory Allocation Failed\n");
                }
		printf("File Size from User is %d\r\n",queryDetails->fileSize);


                remain_data = queryDetails->fileSize;
                while (((len = recv(sock, &queryDetails->fileContent[index], sizeof(char)*queryDetails->fileSize, 0)) > 0) && (remain_data > 0))
                {
                        index = index + len;
                	remain_data -= len;
                        if(remain_data <=0){
                             break;
                        }	
                }

                /*Write into a File*/
                fwrite(queryDetails->fileContent, sizeof(char), len, fp);
		printf("Received File Contente From the User is %s\n", queryDetails->fileContent);	
		fclose(fp);	
               return len;			   
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

void error(char *msg) {
	perror(msg);
	exit(0);
}
