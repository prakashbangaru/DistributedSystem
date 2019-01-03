/***********************************************************************************************************************************************/
/* Table clientFileQueryMapping (user_IP_addr [pk], user_port [pk], queryId [pk], clientIndex [pk], fileName, filePath, startIndex, endIndex) */ 
/**********************************************************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#define HOSTNAME "127.0.0.1"
#define PORT 6379
#define TABLE_NAME "clientFileQueryMapping"
#define MAX_LENGTH 1000


/**
 * This function sets the user IP address, user port, user query id, client index, file name, file path, start index and end index.
 * THE user_IP_addr, user_port, queryId, clientIndex IS THE PRIMARY KEY FOR THE clientFileQueryMapping TABLE.
 * @param user_IP_addr is the IP address of the user.
 * @param user_port is the port number that the user connects to the master.
 * @param queryId is the user query identifier.
 * @param clientIndex is the client identifier to whom the query is assigned.
 * @param fileName is the name of the input file.
 * @param filePath is the path to the input file in the filesystem.
 * @param startIndex is the starting index of file content.
 * @param endIndex is the ending index of file content.
 */
void clientFileQueryMapping_set_all(char *user_IP_addr, int user_port, int queryId, char *clientIndex, char *fileName, char *filePath, long startIndex, long endIndex)
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

    /* Set the hash map clientFileQueryMapping:user_IP_addr:user_port:queryId:clientIndex */
    reply = redisCommand(c,"HMSET %s:%s:%d:%d:%s user_IP_addr %s user_port %d queryId %d clientIndex %s fileName %s filePath %s startIndex %ld endIndex %ld", TABLE_NAME, user_IP_addr, user_port, queryId, clientIndex, user_IP_addr, user_port, queryId, clientIndex, fileName, filePath, startIndex, endIndex);
    freeReplyObject(reply);

    redisFree(c);
}


/**
 * This function gets file name, file path, start index and end index for input user IP address, user port, query id and client index.
 * @param user_IP_addr is the IP address of the user that is the input.
 * @param user_port is the port number of user that is the input.
 * @param queryId is the user query identifier that is the input.
 * @param clientIndex is the client identifier, to whom the query is assigned, that is the input.
 * @param fileName is the name of file we want to retrieve.
 * @param filePath is the path (in the filesystem) of file we want to retrieve.
 * @param startIndex is the starting index of file content that we want to retrieve.
 * @param endIndex is the ending index of file content we want to retrieve.
 */
void clientFileQueryMapping_get_all(char *user_IP_addr, int user_port, int queryId, char *clientIndex, char *fileName, char *filePath, long *startIndex, long *endIndex)
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

    /* get all the query information from the hash key clientFileQueryMapping:user_IP_addr:user_port:queryId:clientIndex */
    reply = redisCommand(c,"HGETALL %s:%s:%d:%d:%s", TABLE_NAME, user_IP_addr, user_port, queryId, clientIndex);
    if ( reply->type == REDIS_REPLY_ERROR ) {
        printf( "Error: %s\n", reply->str );
    } 
    else if ( reply->type != REDIS_REPLY_ARRAY ) {
	printf( "Unexpected type: %d\n", reply->type );
    } 
    else {
        int i = 0;
        strcpy(fileName,reply->element[i+9]->str);
	strcpy(filePath,reply->element[i+11]->str);
	//Redis hashes are maps between string fields and string values
	*startIndex = (long) atoi(reply->element[i+13]->str);
	*endIndex = (long) atoi(reply->element[i+15]->str);
    }
    
    freeReplyObject(reply);
    redisFree(c);
}


/* Sample driver code */
int main()
{    
    char *fileName = NULL, *filePath = NULL;
    long *startIndex = NULL, *endIndex = NULL;
    
    printf("\n============== Setting clientFileQueryMapping information 1 ======================\n");
    clientFileQueryMapping_set_all("1.1.1.5", 1115, 3, "client_1", "FileName-1a", "/home/ubuntu/", 0, 10);

    fileName = malloc(MAX_LENGTH);
    filePath = malloc(MAX_LENGTH);
    startIndex = (long*) malloc(sizeof(long));
    endIndex = (long*) malloc(sizeof(long));
    printf("\n============ Getting clientFileQueryMapping information 1 ==============\n");
    clientFileQueryMapping_get_all("1.1.1.5", 1115, 3, "client_1", fileName, filePath, startIndex, endIndex);
    printf(" fileName: %s\n filePath: %s\n startIndex: %ld\n endIndex: %ld\n=================================\n", fileName, filePath, *startIndex, *endIndex);
    free(fileName);
    free(filePath);
    free(startIndex);
    free(endIndex);
    
    printf("\n============== Setting clientFileQueryMapping information 2 ======================\n");
    clientFileQueryMapping_set_all("1.1.2.5", 1125, 4, "client_1", "FileName-1b", "/home/ubuntu/redis-test/", 1000, 3020);

    fileName = malloc(MAX_LENGTH);
    filePath = malloc(MAX_LENGTH);
    startIndex = (long*) malloc(sizeof(long));
    endIndex = (long*) malloc(sizeof(long));
    printf("\n============ Getting clientFileQueryMapping information 2 ==============\n");
    clientFileQueryMapping_get_all("1.1.2.5", 1125, 4, "client_1", fileName, filePath, startIndex, endIndex);
    printf(" fileName: %s\n filePath: %s\n startIndex: %ld\n endIndex: %ld\n=================================\n", fileName, filePath, *startIndex, *endIndex);
    free(fileName);
    free(filePath);
    free(startIndex);
    free(endIndex);

    return 0;
}












