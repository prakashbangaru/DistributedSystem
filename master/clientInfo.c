/******************************************************************************************************/
/*********************** Table clientInfo (IP_addr, port, clientIndex [pk]) **************************/
/*****************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#define HOSTNAME "127.0.0.1"
#define PORT 6379
#define TABLE_NAME "clientInfo"
#define MAX_LENGTH 1000


/**
 * This function sets the IP address and port corresponding to a generated client-index.
 * THE GENERATED clientIndex IS THE PRIMARY KEY FOR THE clientInfo TABLE.
 * @param IP_addr is the IP address of the client.
 * @param port is the port number that the client connects to the master.
 */
void clientInfo_set(char *IP_addr, char *port)
{
    redisContext *c;
    redisReply *reply;
    char *clientIndex=(char*) malloc(MAX_LENGTH*sizeof(char));

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

    /* Increment client counter for generating clientIndex */
    reply = redisCommand(c,"INCR clientCounter");
    //printf("INCR clientCounter: %lld\n", reply->integer);
    sprintf(clientIndex, "%s%lld", "client_", reply->integer);
    freeReplyObject(reply);

    /* Set the hash map clientInfo:clientIndex */
    reply = redisCommand(c,"HMSET %s:%s IP_addr %s port %s clientIndex %s", TABLE_NAME, clientIndex, IP_addr, port, clientIndex);
    //printf("HMSET %s:%s IP_addr:%s port:%s => %s\n", TABLE_NAME, clientIndex, IP_addr, port, reply->str);
    freeReplyObject(reply);

    redisFree(c);
}


/**
 * This function gets the IP address and port corresponding to the input client-index.
 * @param IP_addr is the IP address of the client that we want to retrieve.
 * @param port will contain the port number of the requested client.
 * @param clientIndex is the identifier of the client whose details we want to retrieve.
 */
void clientInfo_get_IP_addr_port(char *IP_addr, char *port, char *clientIndex)
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

    /* get the IP_addr and port from the hash key clientInfo:clientIndex */
    reply = redisCommand(c,"HGETALL %s:%s", TABLE_NAME, clientIndex);
    //printf("HGETALL %s:%s => %s\n", TABLE_NAME, clientIndex, reply->str);
    if ( reply->type == REDIS_REPLY_ERROR ) {
        printf( "Error: %s\n", reply->str );
    } 
    else if ( reply->type != REDIS_REPLY_ARRAY ) {
	printf( "Unexpected type: %d\n", reply->type );
    } 
    else {
        int i = 0;
        strcpy(IP_addr,reply->element[i+1]->str);
	strcpy(port,reply->element[i+3]->str);
    }    
    
    freeReplyObject(reply);
    redisFree(c);
}


/* Sample driver code */
int main()
{    
    char *IP_addr = NULL, *port = NULL;
    
    printf("\n============== Setting client information 1 ======================\n");
    clientInfo_set("127.1.1.1", "1234");

    IP_addr = malloc(MAX_LENGTH);
    port = malloc(MAX_LENGTH);
    printf("\n============ Getting client information 1 ==============\n");
    clientInfo_get_IP_addr_port(IP_addr, port, "client_1");
    printf(" IP_addr: %s\n port: %s\n=================================\n", IP_addr, port);
    free(IP_addr);
    free(port);
    
    printf("\n============== Setting client information 2 ======================\n");
    clientInfo_set("128.1.1.2", "5678");
    
    IP_addr = malloc(MAX_LENGTH);
    port = malloc(MAX_LENGTH);
    printf("\n============ Getting client information 2 ==============\n");
    clientInfo_get_IP_addr_port(IP_addr, port, "client_2");
    printf(" IP_addr: %s\n port: %s\n=================================\n", IP_addr, port);
    free(IP_addr);
    free(port);

    return 0;
}












