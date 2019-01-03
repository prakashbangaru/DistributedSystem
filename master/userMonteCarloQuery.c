/******************************************************************************************************************************************/
/******************* Table userMonteCarloQuery (user_IP_addr [pk], user_port [pk], queryId [pk], numPoints, numClients) *******************/ 
/*****************************************************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#define HOSTNAME "127.0.0.1"
#define PORT 6379
#define TABLE_NAME "userMonteCarloQuery"


/**
 * This function sets the user IP address, user port, query id, number of distribution points and number of clients.
 * THE user_IP_addr, user_port and queryId IS THE PRIMARY KEY FOR THE userMonteCarloQuery TABLE.
 * @param user_IP_addr is the IP address of the user.
 * @param user_port is the port number that the user connects to the master.
 * @param queryId is the identifier of the user query.
 * @param numPoints is the number of distribution points given by the user.
 * @param numClients is the number of clients assigned to handle that input distribution points.
 */
void userMonteCarloQuery_set_all(char *user_IP_addr, int user_port, int queryId, int numPoints, int numClients)
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

    /* Set the hash map userMonteCarloQuery:user_IP_addr:user_port:queryId */
    reply = redisCommand(c,"HMSET %s:%s:%d:%d user_IP_addr %s user_port %d queryId %d numPoints %d numClients %d", TABLE_NAME, user_IP_addr, user_port, queryId, user_IP_addr, user_port, queryId, numPoints, numClients);
    freeReplyObject(reply);

    redisFree(c);
}


/**
 * This function gets number of distribution points and number of clients for input user IP address, user port and query id.
 * @param user_IP_addr is the IP address of the user that is the input.
 * @param user_port is the port number of user that is the input.
 * @param queryId is the user query identifier that is the input.
 * @param numPoints is the number of distribution points for this query that we want to retrieve.
 * @param numClients is the number of clients assigned to the task that we want to retrieve.
 */
void userMonteCarloQuery_get_all(char *user_IP_addr, int user_port, int queryId, int *numPoints, int *numClients)
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

    /* get all the query information from the hash key userMonteCarloQuery:user_IP_addr:user_port:queryId */
    reply = redisCommand(c,"HGETALL %s:%s:%d:%d", TABLE_NAME, user_IP_addr, user_port, queryId);
    if ( reply->type == REDIS_REPLY_ERROR ) {
        printf( "Error: %s\n", reply->str );
    } 
    else if ( reply->type != REDIS_REPLY_ARRAY ) {
	printf( "Unexpected type: %d\n", reply->type );
    } 
    else {
        int i = 0;
	//Redis hashes are maps between string fields and string values
	*numPoints = atoi(reply->element[i+7]->str);
	*numClients = atoi(reply->element[i+9]->str);
    }
    
    freeReplyObject(reply);
    redisFree(c);
}


/* Sample driver code */
/*int main()
{    
    int *numPoints = NULL, *numClients = NULL;
    
    printf("\n============== Setting userMonteCarloQuery information 1 ======================\n");
    userMonteCarloQuery_set_all("1.1.1.5", 1020, 1, 1000, 4);
    
    numPoints = (int*) malloc(sizeof(int));
    numClients = (int*) malloc(sizeof(int));
    printf("\n============ Getting userMonteCarloQuery information 1 ==============\n");
    userMonteCarloQuery_get_all("1.1.1.5", 1020, 1, numPoints, numClients);
    printf(" numPoints: %d\n numClients: %d\n=================================\n", *numPoints, *numClients);
    free(numClients);
    free(numPoints);
    
    printf("\n============== Setting userMonteCarloQuery information 2 ======================\n");
    userMonteCarloQuery_set_all("11.1.1.2", 2001, 2, 1500, 100);

    numClients = (int*) malloc(sizeof(int));
    numPoints = (int*) malloc(sizeof(int));
    printf("\n============ Getting userMonteCarloQuery information 2 ==============\n");
    userMonteCarloQuery_get_all("11.1.1.2", 2001, 2, numPoints, numClients);
    printf(" numPoints: %d\n numClients: %d\n=================================\n", *numPoints, *numClients);
    free(numClients);
    free(numPoints);

    return 0;
}*/












