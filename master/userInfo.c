/******************************************************************************************************/
/*********************** Table userInfo (IP_addr, port, userIndex [pk]) **************************/
/*****************************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#define HOSTNAME "127.0.0.1"
#define PORT 6379
#define TABLE_NAME "userInfo"
#define MAX_LENGTH 1000


/**
 * This function sets the IP address and port corresponding to a generated user-index.
 * THE GENERATED userIndex IS THE PRIMARY KEY FOR THE userInfo TABLE.
 * @param IP_addr is the IP address of the user.
 * @param port is the port number that the user connects to the master.
 */
void userInfo_set(char *IP_addr, char *port)
{
    redisContext *c;
    redisReply *reply;
    char *userIndex=(char*) malloc(MAX_LENGTH*sizeof(char));

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

    /* Increment user counter for generating userIndex */
    reply = redisCommand(c,"INCR userCounter");
    //printf("INCR userCounter: %lld\n", reply->integer);
    sprintf(userIndex, "%s%lld", "user_", reply->integer);
    freeReplyObject(reply);

    /* Set the hash map userInfo:userIndex */
    reply = redisCommand(c,"HMSET %s:%s IP_addr %s port %s userIndex %s", TABLE_NAME, userIndex, IP_addr, port, userIndex);
    //printf("HMSET %s:%s IP_addr:%s port:%s => %s\n", TABLE_NAME, userIndex, IP_addr, port, reply->str);
    freeReplyObject(reply);

    redisFree(c);
}


/**
 * This function gets the IP address and port corresponding to the input user-index.
 * @param IP_addr is the IP address of the user that we want to retrieve.
 * @param port will contain the port number of the requested user.
 * @param userIndex is the identifier of the user whose details we want to retrieve.
 */
void userInfo_get_IP_addr_port(char *IP_addr, char *port, char *userIndex)
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

    /* get the IP_addr and port from the hash key userInfo:userIndex */
    reply = redisCommand(c,"HGETALL %s:%s", TABLE_NAME, userIndex);
    //printf("HGETALL %s:%s => %s\n", TABLE_NAME, userIndex, reply->str);
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
    
    printf("\n============== Setting user information 1 ======================\n");
    userInfo_set("125.1.1.1", "4567");

    IP_addr = malloc(MAX_LENGTH);
    port = malloc(MAX_LENGTH);
    printf("\n============ Getting user information 1 ==============\n");
    userInfo_get_IP_addr_port(IP_addr, port, "user_1");
    printf(" IP_addr: %s\n port: %s\n=================================\n", IP_addr, port);
    free(IP_addr);
    free(port);
    
    printf("\n============== Setting user information 2 ======================\n");
    userInfo_set("126.1.1.2", "1357");
    
    IP_addr = malloc(MAX_LENGTH);
    port = malloc(MAX_LENGTH);
    printf("\n============ Getting user information 2 ==============\n");
    userInfo_get_IP_addr_port(IP_addr, port, "user_2");
    printf(" IP_addr: %s\n port: %s\n=================================\n", IP_addr, port);
    free(IP_addr);
    free(port);

    return 0;
}












