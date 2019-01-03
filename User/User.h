#define USER_PORT 2221
#define BUF_LENGTH 1024
#define FILE_NAME_LENGTH 256
#define QUERYID_LENGTH 12
#define QUERYDETAIL_LENGTH 24
#define MONTECARLO_MESSAGE_TYPE 1
#define FILE_SORT_MESSAGE_TYPE 2
#define QUERY_MESSAGE_TYPE 4

#define PADDING_BYTE 4

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

/*Solution Identity*/
typedef struct{
        int32_t queryType;
        int32_t queryId;
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
void* clientListeningFunction();
void* connection_handler(void* sock);
//void connection_handler(int sock);
int sendFileQuery(int sockfd, QueryDetailsType *queryDetails);
void startFileMergeSort(UserFileQueryTable userFileQuery);
void monteCarloSoultionHandler(UserMonteCarloQueryTable userMonteCarloQuery);
