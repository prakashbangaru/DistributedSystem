/******************************************************************************************************************************************/
/*********** Table userFileQuery (user_IP_addr [pk], user_port [pk], queryId [pk], fileName, filePath, numClients, fileSize) **************/ 
/*****************************************************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#define HOSTNAME "127.0.0.1"
#define PORT 6379
#define TABLE_NAME "userFileQuery"
#define MAX_LENGTH 1000


/**
 * This function sets the user IP address, user port, query id, file name, file path, number of clients and file size.
 * THE user_IP_addr, user_port and queryId IS THE PRIMARY KEY FOR THE userFileQuery TABLE.
 * @param user_IP_addr is the IP address of the user.
 * @param user_port is the port number that the user connects to the master.
 * @param queryId is the identifier of the user query.
 * @param fileName is the name of the input file.
 * @param filePath is the path to the input file in the filesystem.
 * @param numClients is the number of clients assigned to handle that input file.
 * @param fileSize is the size of the input file.
 */
void userFileQuery_set_all(char *user_IP_addr, int user_port, int queryId, char *fileName, char *filePath, int numClients, int fileSize)
{
    redisContext *c;
    redisReply *reply;

    /* HOSTNAME and PORT of the redis-server */
    c = redisConnect(HOSTNAME, PORT);

    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    /* PING server */
    reply = redisCommand(c,"PING");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);

    /* Set the hash map userFileQuery:user_IP_addr:user_port:queryId */
    reply = redisCommand(c,"HMSET %s:%s:%d:%d user_IP_addr %s user_port %d queryId %d fileName %s filePath %s numClients %d fileSize %d", TABLE_NAME, user_IP_addr, user_port, queryId, user_IP_addr, user_port, queryId, fileName, filePath, numClients, fileSize);
    freeReplyObject(reply);

    redisFree(c);
}


/**
 * This function gets file name, file path, number of clients and file size for input user IP address, user port and query id.
 * @param user_IP_addr is the IP address of the user that is the input.
 * @param user_port is the port number of user that is the input.
 * @param queryId is the user query identifier that is the input.
 * @param fileName is the name of file we want to retrieve.
 * @param filePath is the path (in the filesystem) of file we want to retrieve.
 * @param numClients is the number of clients assigned to the task that we want to retrieve.
 * @param fileSize is the size of the file we want to retrieve.
 */
void userFileQuery_get_all(char *user_IP_addr, int user_port, int queryId, char *fileName, char *filePath, int *numClients, int *fileSize)
{
    redisContext *c;
    redisReply *reply;

    /* HOSTNAME and PORT of the redis-server */
    c = redisConnect(HOSTNAME, PORT);
    
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    /* PING server */
    reply = redisCommand(c,"PING");
    printf("PING: %s\n", reply->str);
    freeReplyObject(reply);

    /* get all the query information from the hash key userFileQuery:user_IP_addr:user_port:queryId */
    reply = redisCommand(c,"HGETALL %s:%s:%d:%d", TABLE_NAME, user_IP_addr, user_port, queryId);
    if ( reply->type == REDIS_REPLY_ERROR ) {
        printf( "Error: %s\n", reply->str );
    } 
    else if ( reply->type != REDIS_REPLY_ARRAY ) {
	printf( "Unexpected type: %d\n", reply->type );
    } 
    else {
        int i = 0;
        strcpy(fileName,reply->element[i+7]->str);
	strcpy(filePath,reply->element[i+9]->str);
	//Redis hashes are maps between string fields and string values
	*numClients = atoi(reply->element[i+11]->str);
	*fileSize = atoi(reply->element[i+13]->str);
    }
    
    freeReplyObject(reply);
    redisFree(c);
}


/* Sample driver code */
int main()
{    
    char *fileName = NULL, *filePath = NULL;
    int *numClients = NULL, *fileSize = NULL;
    
    printf("\n============== Setting userFileQuery information 1 ======================\n");
    userFileQuery_set_all("120.1.1.5", 1212, 1, "FileName-001", "/home/admin/", 13, 25);

    fileName = malloc(MAX_LENGTH);
    filePath = malloc(MAX_LENGTH);
    numClients = (int*) malloc(sizeof(int));
    fileSize = (int*) malloc(sizeof(int));
    printf("\n============ Getting userFileQuery information 1 ==============\n");
    userFileQuery_get_all("120.1.1.5", 1212, 1, fileName, filePath, numClients, fileSize);
    printf(" fileName: %s\n filePath: %s\n numClients: %d\n fileSize: %d\n=================================\n", fileName, filePath, *numClients, *fileSize);
    free(fileName);
    free(filePath);
    free(numClients);
    free(fileSize);
    
    printf("\n============== Setting userFileQuery information 2 ======================\n");
    userFileQuery_set_all("121.2.1.6", 1331, 2, "FileName-002", "/home/admin/redis-admin/", 100, 650);

    fileName = malloc(MAX_LENGTH);
    filePath = malloc(MAX_LENGTH);
    numClients = (int*) malloc(sizeof(int));
    fileSize = (int*) malloc(sizeof(int));
    printf("\n============ Getting userFileQuery information 2 ==============\n");
    userFileQuery_get_all("121.2.1.6", 1331, 2, fileName, filePath, numClients, fileSize);
    printf(" fileName: %s\n filePath: %s\n numClients: %d\n fileSize: %d\n=================================\n", fileName, filePath, *numClients, *fileSize);
    free(fileName);
    free(filePath);
    free(numClients);
    free(fileSize);

    return 0;
}












