/* 
ize
 * usage: user.out <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include "User.h"

#define QUERY_COUNT_FILE "queryCountFile.txt"

#define MAX_QUERY_COUNT 5000

int subSolutionCount[MAX_QUERY_COUNT]; 

typedef struct{
    int32_t userPortNumber;
    int32_t queryId;   
}Query;


void systemInit(int *queryIndex){
  FILE *fp = fopen(QUERY_COUNT_FILE, "r");
  if(fp == NULL){
       error("file OPen Failed at queryCountFile.txt");
   }
   if ( fscanf (fp,"%d",queryIndex) != EOF ){
      printf("Your Query Starts With %d",*queryIndex);
   }
   fclose(fp);
}

void updateQueryIndex(int queryIndex){
  char queryCountValue[15];
  
  memset(queryCountValue,'\0',sizeof(char)*15);
  FILE *fp = fopen(QUERY_COUNT_FILE,"w");
   
   if(fp == NULL){
       error("file OPen Failed to UPdate  queryCountFile.txt");
   }
   sprintf(queryCountValue,"%d",queryIndex); 
    fwrite(queryCountValue, sizeof(char), strlen(queryCountValue), fp);
     fclose(fp);
   

}
int main(int argc, char **argv) {
	int  portno =0;
	int  n =0;
	int sockfd=0;          
	struct sockaddr_in serveraddr;
	struct hostent *server=NULL;
	struct hostent *clientIP=NULL;
	char *hostname=NULL;
	char buf[BUF_LENGTH];
	int32_t queryType=0;
	int32_t numberOfDistributionPoints=0;
	char fileName[FILE_NAME_LENGTH];
	/*For USer- User Type is 1 and for client User Type is 2*/
	int32_t userType = 1;
	int queryIndex =0;

	memset(fileName,'\0',FILE_NAME_LENGTH*sizeof(char));
	memset(buf,'\0',BUF_LENGTH*sizeof(char));
         //BVP remove

         printf("Size Check %d\n",sizeof(QueryDetailsType));
 	/* check command line arguments */
	if (argc != 3) {
		fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
		exit(0);
	}
	hostname = argv[1];
	portno = atoi(argv[2]);

        systemInit(&queryIndex);
        printf("Query Index Count = %d\n",queryIndex);
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

	/*Spawn Thread for client to listen to the Servere for the Solution to the Query Submitted*/
	pthread_t thread_id;
	if( pthread_create( &thread_id , NULL ,  clientListeningFunction ,NULL) < 0){
		perror("could not create thread");
		return 1;
	}

	while(true){
		QueryDetailsType queryDetails;
		memset(&queryDetails,'\0',sizeof(queryDetails));
		strcpy(queryDetails.ipAddress,"xxx.xxx.xxx.xxx"); //Fill dummy IP address
		queryDetails.portNumber = USER_PORT; // Fill Client Bind Port Number
		queryDetails.queryId = queryIndex;
		printf("Enter Query Type \n");
		printf("1. MonteCarlo \n");
		printf("2. Sort the File \n");
                printf("4. QueryID\n"); 
		printf("5. Exit \n");
		scanf("%d",&queryType);
		/* socket: create the socket */
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0) 
			error("ERROR opening socket");

		if(queryType == FILE_SORT_MESSAGE_TYPE || queryType == MONTECARLO_MESSAGE_TYPE || queryType == QUERY_MESSAGE_TYPE){
			queryDetails.queryType = queryType; // Fill Query Type

			/* connect: create a connection with the server */
			n = connect(sockfd, &serveraddr, sizeof(serveraddr));
			if(n < 0){
				error("ERROR connecting");
			}

			/*Send User Type to the Server*/
			n= write(sockfd,&userType, sizeof(userType)); 
			if (n < 0){ 
				error("ERROR writing User Type  to the socket");
			}
		}

		if(queryType == MONTECARLO_MESSAGE_TYPE){
			printf("Enter the Number of Distribution Points\n");
			scanf("%d",&numberOfDistributionPoints);
			//Fill Query Detail for MonteCarlo
			sprintf(queryDetails.queryDetail,"%d",numberOfDistributionPoints);
			n = write(sockfd,(void*) &queryDetails, sizeof(QueryDetailsType));
			if (n < 0){ 
				error("ERROR writing numberOfDistributionPoints to socket");
			}
                        timing_start();
  
                        printf("Query Id: %d is Submitted\n",queryDetails.queryId);
		}
		else if(queryType == FILE_SORT_MESSAGE_TYPE){
			printf("Enter the FileName\n");
			scanf("%s",queryDetails.queryDetail);
			sendFileQuery(sockfd,&queryDetails);
                        timing_start();
                        printf("Query Id: %d is Submitted\n",queryDetails.queryId);
		}
		else if(queryType == QUERY_MESSAGE_TYPE){
                       double piValue=0.0;
                       scanf("%d",&queryDetails.queryId);
                       
			n = write(sockfd,(void*) &queryDetails, sizeof(QueryDetailsType));
			if (n < 0){ 
				error("ERROR writing Query Submission to the MAster");
			}
                        timing_start();
                       printf("Query Id: %d is Submitted\n",queryDetails.queryId);
                        if( (n= recv(sockfd , &piValue , sizeof(double) , 0)) > 0 ){
                           if(piValue!=0.0){
                              printf("PiValue for the Query %d : %lf\n",queryDetails.queryId,piValue);
                           }  
                        }
                        if(piValue== 0.0){
                           printf("failed to Compute the Pi Value\n");
                        }
                         timing_stop();
                         print_timing();
 
                        close(sockfd);
                        
                }
		else{
			/*Have to code that system has to wait if solution has to be received for the Previous Submitted  Query*/
                        close(sockfd);
			printf("System Exit.Bye\n");
			break;
		}
                //close(sockfd);
		queryIndex++;
                updateQueryIndex(queryIndex);
	}

	return 0;
}

void error(char *msg) {
	perror(msg);
	exit(0);
}

int sendFileQuery(int sockfd, QueryDetailsType *queryDetails){
	int fp=-1;
	struct stat file_stat;
	int n =0;
	long offset =0;
	long remain_data =0;
	long sent_bytes=0;
        int32_t fileLineCount =0;


	fp = open(queryDetails->queryDetail, O_RDONLY);
	if(fp == -1){
		error("ERROR in Opening the File");
	}

	/* Get file stats */
	if (fstat(fp, &file_stat) < 0){
		fprintf(stderr, "Error fstat --> %s", strerror(errno));
		exit(EXIT_FAILURE);
	} 
  
        queryDetails->fileSize = (int32_t)file_stat.st_size-1;

        // Send the FileSort Query 
	n = write(sockfd,(void*)queryDetails, sizeof(QueryDetailsType));
	if (n < 0){
		fprintf(stderr, "Error on sending FileSort Query %s", strerror(errno));
		exit(EXIT_FAILURE);
	}

        queryDetails->fileContent = (char*)malloc(sizeof(char)*(queryDetails->fileSize));

        if(queryDetails->fileContent!=NULL){
        	memset(queryDetails->fileContent,0,sizeof(char)*queryDetails->fileSize);
        }
        else{
        	error("File Content Memory Allocation Failed\n");
        }

	fprintf(stdout, "File Size: %d bytes\n",queryDetails->fileSize);
        /*Read the File Content*/
        read(fp,queryDetails->fileContent, queryDetails->fileSize);
        close(fp);
        //BVP
        /*Sending the File Content*/
        n = write(sockfd, (void*)queryDetails->fileContent , sizeof(char)*queryDetails->fileSize);
	if (n < 0){
		fprintf(stderr, "Error on sending File Size %s", strerror(errno));
		exit(EXIT_FAILURE);
	}
         //BVP
        printf("Sent %d Bytes\n",n);
        printf("File Content Sent To the Master %s\n",queryDetails->fileContent);

        #if 0
	/* Sending file data */
	while (((sent_bytes = sendfile(sockfd, fp, &offset, BUFSIZ)) > 0) && (remain_data > 0))
	{
		fprintf(stdout, "1. Server sent %ld bytes from file's data, offset is now : %ld and remaining data = %ld\n", sent_bytes, offset, remain_data);
		remain_data -= sent_bytes;
		fprintf(stdout, "2. Server sent %ld bytes from file's data, offset is now : %ld and remaining data = %ld\n", sent_bytes, offset, remain_data);
	}
        close(fp);
       #endif

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
	int client_desc =0;
        int *server_desc=NULL; 
        int c=0;
	struct sockaddr_in server , client;
	//Create socket
	client_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (client_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");
	//Prepare the sockaddr_in structure
	client.sin_family = AF_INET;
	client.sin_addr.s_addr = INADDR_ANY;
	client.sin_port = htons(USER_PORT);
	//Bind
	if( bind(client_desc,(struct sockaddr *)&client , sizeof(client)) < 0)
	{
		//print the error message
		perror("bind failed at Client side. Error");
		exit(EXIT_FAILURE);
	}
	puts("bind done");
	//Listen
	listen(client_desc , 1);
	//Accept and incoming connection
	puts("Waiting for  Solution Data from Server..");
	c = sizeof(struct sockaddr_in);

	while(1){
                server_desc = (int*)malloc(sizeof(int));
          	(*server_desc) = accept(client_desc, (struct sockaddr *)&server, (socklen_t*)&c);
		printf("Connection accepted\n");
                /*Spawn Thread to get solution from the Master*/
                pthread_t thread_id;
               if( pthread_create(&thread_id,NULL,connection_handler ,(void*)server_desc) < 0){
                   perror("could not create thread");
                   exit(0);
              }
	}
}

void* connection_handler(void* sockArg){
        int sock = *((int*)(sockArg));
	int read_size=0;
	int32_t queryType=0;
        int32_t messageType=0;
        UserMonteCarloQueryTable userMonteCarloQuery;


	if( (read_size = recv(sock , &messageType , sizeof(messageType) , 0)) > 0 )
	{
		if(messageType == MONTECARLO_MESSAGE_TYPE){ //MonteCarlo
			// Receive Number of Distribution Points
			printf("Received MonteCarlo Solution\r\n");
			if( (read_size = recv(sock , &userMonteCarloQuery , sizeof(UserMonteCarloQueryTable) , 0)) > 0 ){
                                  monteCarloSoultionHandler(userMonteCarloQuery);
			}
		}
		if(messageType == FILE_SORT_MESSAGE_TYPE){ //File Sorted
			printf("Received sorted File solution\r\n");
			receiveFile(sock);
		}
	}
        free(sockArg);
}

void monteCarloSoultionHandler(UserMonteCarloQueryTable userMonteCarloQuery){
     int  numberOfSubSolution=0;
     FILE *fp =NULL;
     char fileName[FILE_NAME_LENGTH];
     char stringCircularPoint[FILE_NAME_LENGTH];
     int index=0;
     int32_t totalCircularPoint =0;
     int32_t clientsCircularPoint=0;
     char space[2]=" \0";
     double piValue=0.0;

     memset(fileName,'\0',FILE_NAME_LENGTH); 
     memset(stringCircularPoint,'\0',FILE_NAME_LENGTH);

     /*If First Solution is Received*/ 
     if(subSolutionCount[userMonteCarloQuery.queryId]== 0){
          subSolutionCount[userMonteCarloQuery.queryId] = userMonteCarloQuery.numberOfClients;
          subSolutionCount[userMonteCarloQuery.queryId]--;
     } 
     else{
          subSolutionCount[userMonteCarloQuery.queryId]--;
     }  
 
     sprintf(fileName,"%d",userMonteCarloQuery.queryId);
      /*Write the File Content into a File*/
     fp = fopen(fileName, "a");
     if(fp == NULL){
            error("file OPen Failed at monteCarloSoultionHandler");
      }
     printf("Received Montecarlo Solution %d %d\n",userMonteCarloQuery.totalDistributionCount,userMonteCarloQuery.totalCircularCount);
     sprintf(stringCircularPoint,"%d%s",userMonteCarloQuery.totalCircularCount,space);
     printf("Circular Point as String  %s\n",stringCircularPoint);
     fwrite(stringCircularPoint, sizeof(char), strlen(stringCircularPoint), fp);
     fclose(fp);     
      
     if(subSolutionCount[userMonteCarloQuery.queryId] == 0){
           fp = fopen(fileName,"r");

           if(fp == NULL){
                  error("file OPen Failed at monteCarloSolutionHandler Readble Mode");
            }

     	   for(index=0;index < userMonteCarloQuery.numberOfClients; index++){
             
             memset(stringCircularPoint,'\0',FILE_NAME_LENGTH);
             
             if ( fscanf (fp,"%s",stringCircularPoint) != EOF ){
                 printf("String totalCircularPoint Read From File %sspaceCheck\n",stringCircularPoint);
                 clientsCircularPoint = atoi(stringCircularPoint);
                 totalCircularPoint +=clientsCircularPoint;
                 printf(" totalCircularPoint =%d clientsCircularPoint =%d\n",totalCircularPoint,clientsCircularPoint);
             }
             
     	  }
          fclose(fp);
          piValue = (double) ((totalCircularPoint * 4) /(double)(userMonteCarloQuery.totalDistributionCount));
          printf("Computed Pie Value is %lf For the Query %d\n",piValue, userMonteCarloQuery.queryId);
           timing_stop();
           print_timing();

     }
            
}

int receiveFile(int socketMaster){
     UserFileQueryTable userFileQuery;
     FILE *fp=NULL;
     int userSocket =0;
     int32_t messageType= FILE_SORT_MESSAGE_TYPE;
     int n=0;
     int readSize=0;
      /*Receive User file Query Sub-Solution from Master*/
        if( (readSize = recv(socketMaster , (void*)&userFileQuery , sizeof(UserFileQueryTable) , 0)) > 0 ){
                printf("Received File SortQuery Solution from  Master, File Size is %d Number Of client %d File Name:%s\n",userFileQuery.fileSize,userFileQuery.numberOfClients,userFileQuery.fileName);
                
                userFileQuery.fileContent = malloc(sizeof(char)*userFileQuery.fileSize);
                if(userFileQuery.fileContent == NULL){
                    error("Memory Allocation Failed for File Content to be received from Master\n");
                }
                memset(userFileQuery.fileContent,'\0',sizeof(char)*userFileQuery.fileSize);
                /*Receive File Content*/
                if( (readSize = recv(socketMaster , (void*)userFileQuery.fileContent , sizeof(char)*userFileQuery.fileSize , 0)) > 0 ){
                        //printf("File content: %s \n Received Successfully From the  Master\n",userFileQuery.fileContent);
                        close(socketMaster);
                        /*Write the File Content into a File*/
                	fp = fopen(userFileQuery.fileName, "w");
                	if(fp == NULL){
                        	error("file OPen Failed");
                	}
                        fwrite(userFileQuery.fileContent, sizeof(char), readSize, fp);
                        fclose(fp);
                }
                 /*Received First Sorted Chunk File */
                 if(subSolutionCount[userFileQuery.queryId]==0){
			subSolutionCount[userFileQuery.queryId]=userFileQuery.numberOfClients;
                 }

                 if(subSolutionCount[userFileQuery.queryId]>0){
                 	subSolutionCount[userFileQuery.queryId]--;
               
                        if(userFileQuery.numberOfClients == 1){
                          /*Rename the File Name from _0 to originalName_0 BVP*/
                           printf("the Sorted File For Query Id %d is: %s\n",userFileQuery.queryId,userFileQuery.fileName);
                         }   
                         if (userFileQuery.numberOfClients > 1){
                                /*If All the Chunk File from the Client is received Then Do File Merge Sort*/
                 		if(subSolutionCount[userFileQuery.queryId]==0){
					startFileMergeSort(userFileQuery);
                                         timing_stop();
                                         print_timing();

                                }
                         }                            

                 }
      }
}

void startFileMergeSort( UserFileQueryTable userFileQuery){
      char *sort ="SORT";
      int index=0;
      char *token=NULL;
      char tempFileName[FILE_NAME_LENGTH];
      char file_1[FILE_NAME_LENGTH];
      char file_2[FILE_NAME_LENGTH];
      char sortedFileName[FILE_NAME_LENGTH];
      char underScore[2]="_";
      char zero[2]="0";
      char one[2]="1";
      memset(tempFileName,'\0',FILE_NAME_LENGTH);
      memset(file_1,'\0',FILE_NAME_LENGTH);
      memset(file_2,'\0',FILE_NAME_LENGTH);
      memset(sortedFileName,'\0',FILE_NAME_LENGTH);
      /*Get the Exact File Name By removing the Index*/
      strcpy(tempFileName,userFileQuery.fileName);
      token = strtok(tempFileName,underScore);
      /*SORT_FILENAME*/
      sprintf(sortedFileName,"%s%s%s",sort,underScore,token);
      /*FILNAME_0*/
      sprintf(file_1,"%s%s%s",token,underScore,zero);
      /*FILNAME_1*/
      sprintf(file_2,"%s%s%s",token,underScore,one);

      /*First do MergeSort the Filename_0 and Filename_1*/
       printf("Merge Sort the Two File %s%s\n",file_1,file_2);
       fileMergeSort(file_1,file_2,sortedFileName);

      for(index=2; index< userFileQuery.numberOfClients; index++){
                     memset(file_1,'\0',FILE_NAME_LENGTH);
                     /*FileNAME_IndexNumber*/
                     sprintf(file_1,"%s%s%d",token,underScore,index);
                     printf("Merge Sort the Two File %s%s\n",sortedFileName,file_1);
                     fileMergeSort(sortedFileName,file_1,sortedFileName);
      }
       printf("Sorted File for the Query Id : %d is : %s\n",userFileQuery.queryId,sortedFileName); 
       subSolutionCount[userFileQuery.queryId]=0;

}

