#include <netinet/in.h>
#include <stdbool.h>

#define FILE_NAME_LENGTH 256
#define BUF_LENGTH 1024
#define SERVER_PORT 9999
#define MAX_USER 10
#define MAX_CLIENT 10
#define QUERYDETAIL_LENGTH 256

#define MONTECARLO_MESSAGE_TYPE 1
#define FILE_SORT_MESSAGE_TYPE 2
#define CLIENT_IDENTITY_MESSAGE_TYPE 3
#define QUERY_MESSAGE_TYPE 4
#define USER_USER_TYPE 1
#define CLIENT_USER_TYPE 2

#define PADDING_BYTE 4

#define MAX_LINE_LENGTH 100

typedef struct{
  char ipAddress[INET_ADDRSTRLEN];
  int32_t portNumber;
  int32_t queryType;
  int32_t queryId;
  int32_t fileSize;
  char queryDetail[QUERYDETAIL_LENGTH];
  char *fileContent;
  char bytePadding[PADDING_BYTE];

}QueryDetailsType;


typedef struct{
   char userIpAddress[INET_ADDRSTRLEN];
   int32_t userPortNumber;
   int32_t queryId;
   int32_t totalDistributionCount;
   int32_t clientsDistributionCount;
   int32_t totalCircularCount;
   int32_t numberOfClients; /*Number of clients handles the given File Query*/
}UserMonteCarloQueryTable;


typedef struct{
  char ipAddress[INET_ADDRSTRLEN];
  int sock;
}ThreadAttribute;

/*Client Identity*/
typedef struct{
        char ipAddress[INET_ADDRSTRLEN];
        int32_t portNumber;
}ClientIdentityType;

/*DataBase Purupose Structure Related to Files - Start*/

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
  char fileName[FILE_NAME_LENGTH];
  char *sortedFileContent;
  char clientIpAddress[INET_ADDRSTRLEN];
  int32_t clientPortNumber;
  int32_t startIndex; /*start Index of File content*/
  int32_t endIndex;  /*End Index of File Content*/
}ClientFileQueryMappingTable;


void sendUserMonteCarloSolution(UserMonteCarloQueryTable *userMonteCarloQuery);

void getUserMonteCarloQuery(UserMonteCarloQueryTable *userMonteCarloQuery,QueryDetailsType *queryDetails,\
                            int numberOfClients,int32_t clientDistributionPoint);

void *connection_handler(void *threadAttrParam);
void clientHandler (ThreadAttribute *threadAttr);
void addClient(int clientSocket,char *clientIpAddress);
void removeClient(char * clientIpAddress,int32_t portNumber);
void updateClientDetails(int clientDetailIndex);
bool isClientExists(char *clientIpAddress,int32_t portNumber);
void userHandler(QueryDetailsType *queryDetails,int sock,char *ipAddress);
void userMontecarloHandler(QueryDetailsType *queryDetails);
void userFileHandler(QueryDetailsType *queryDetails, int sock);
int connectToClient(char *hostName,int32_t clientPortNumber);
void getWordList(char **wordList, char *fileName,int32_t numberOfLines);
int getEndIndex(int fileDataIndex,char *fileContent);
void getUserFileQuery(UserFileQueryTable *userFileQuery,QueryDetailsType *queryDetails, int numberOfClients);
//void getUserFileQuery(UserFileQueryTable *userFileQuery,QueryDetailsType *queryDetails, int totalFileSize,\
                      char *fileContent,int numberOfClients);
int32_t receiveFileFromUser(int sock,QueryDetailsType *queryDetails);
//int32_t receiveFileFromUser(int sock, char *fileNameServerSide, char *fileContent);
void error(char *msg);
int receiveData(int sock,char *message);

char* sortFileContent(char* fileName,int *fileSize);
void convertToOneD(char *oneDArr, char **words, int count_lines);
void getWords(char *filename, char **words);
void sort(char **arr, int num);
int compareTwoChars (const void * a, const void * b);
char* getLower(char *str);
long getNumberOfLines(char *filename);

void processMonteCarlo(int32_t num_points, int *num_circle);
void monteCarloHandler(int sock);

void clientMonteCarloHandler(int clientSocket);

void userQueryHandler(QueryDetailsType *queryDetails,int sock);
