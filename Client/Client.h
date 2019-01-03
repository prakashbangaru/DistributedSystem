#include <netinet/in.h>
#define CLIENT_PORT 1111
#define BUF_LENGTH 1024
#define FILE_NAME_LENGTH 256
#define QUERYID_LENGTH 12
#define QUERYDETAIL_LENGTH 24
#define SOLUTIONDETAIL_LENGTH 24

#define MONTECARLO_MESSAGE_TYPE 1
#define FILE_SORT_MESSAGE_TYPE 2
#define CLIENT_IDENTITY_MESSAGE_TYPE 3

#define CLIENT_USER_TYPE 2				
	
#define MASTER_HOST_NAME_LENGTH 256
# define MAX_LINE_LENGTH 100
/*Query Identity*/
typedef struct{
        char ipAddress[INET_ADDRSTRLEN];
        int32_t portNumber;
        int32_t queryType;
        int32_t queryId;
        char queryDetail[QUERYDETAIL_LENGTH];
}QueryDetailsType;

/*Solution Identity*/
typedef struct{
        char ipAddress[INET_ADDRSTRLEN];
        int32_t portNumber;
        int32_t queryType;
        int32_t queryId;
        char queryDetail[QUERYDETAIL_LENGTH];
        int32_t subTaskId;
        char solutionDetail[SOLUTIONDETAIL_LENGTH];
}SolutionIdentityType;


typedef struct{
   char userIpAddress[INET_ADDRSTRLEN];
   int32_t userPortNumber;
   int32_t queryId;
   int32_t fileSize;
   char fileName[FILE_NAME_LENGTH];
   char *fileContent;
   int32_t numberOfClients; /*Number of clients handles the given File Query*/
}UserFileQueryTable;

typedef struct{
   char userIpAddress[INET_ADDRSTRLEN];
   int32_t userPortNumber;
   int32_t queryId;
   int32_t totalDistributionCount;
   int32_t clientsDistributionCount;
   int32_t totalCircularCount;
   int32_t numberOfClients; /*Number of clients handles the given File Query*/
}UserMonteCarloQueryTable;



void error(char *msg);
int sendFile(int sockfd, char *fileName);
int sendData(int sock, char *message);
void* clientListeningFunction(void *arg);
void connectionHandler(int sock);
void fileHandler(int sock);
int receiveFile(int sock);
int receiveData(int sock,char *message);


/*Functions Related to File Sort*/

//int sortFileContent(char* fileName,char** sortedFileContent);
char* sortFileContent(char* fileName,int *fileSize);
void convertToOneD(char *oneDArr, char **words, int count_lines);
void getWords(char *filename, char **words);
void sort(char **arr, int num);
int compareTwoChars (const void * a, const void * b);
char* getLower(char *str);
long getNumberOfLines(char *filename);
void processMonteCarlo(int32_t num_points, int *num_circle);
void monteCarloHandler(int sock);

