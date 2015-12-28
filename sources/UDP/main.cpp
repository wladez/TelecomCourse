#include <QCoreApplication>
#include <pthread.h>
#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include "server.h"


int main(int argc, char *argv[])
{
    int sock, port_num;
    struct sockaddr_in serv_addr;
    socklen_t clilen;
    pid_t pid;

    sock=socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }
    bzero((char*)&serv_addr, sizeof(serv_addr));
    //port_num=12345;
    port_num = atoi(argv[1]);

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(port_num);

    if(bind(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
        perror("ERROR on binding");
        exit(1);
    }

    printf("UDP Server Waiting for client on port 12345\n");

    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

    pthread_t serv_thread;
    if (pthread_create(&serv_thread, NULL, server_handler, NULL) != 0) {
        printf("Error while creating thread for server\n");
    }

    while (1) {
        pthread_t thread;
        user client = new_connection(sock);
        pid=pthread_create(&thread, NULL, doprocessing, (void*)&client);
        if (pid!=0){
            printf("Error while creating new thread\n");
            //break;
        }
    } /* end of while */
    return 0;

}
