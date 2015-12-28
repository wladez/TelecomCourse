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
//#include "user.h"


int main(int argc, char *argv[])
{
    int sock, newsock, port_num, n;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    pid_t pid;

    sock=socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
          {
          perror("ERROR opening socket");
          exit(1);
          }
    bzero((char*)&serv_addr, sizeof(serv_addr));
    port_num=12345;

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(port_num);

    if(bind(sock,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
        perror("ERROR on binding");
        exit(1);
    }

    listen(sock,10);
    clilen=sizeof(cli_addr);
    printf("TCP Server Waiting for client on port 12345\n");

    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

    pthread_t serv_thread;
    if (pthread_create(&serv_thread, NULL, server_handler, NULL) != 0) {
            printf("Error while creating thread for server\n");
        }

    while (1) {
         pthread_t thread;
         newsock = accept(sock, (struct sockaddr *) &cli_addr, &clilen);

         if (newsock < 0) {
            perror("ERROR on accept");
            close(sock);
            exit(1);
         }
//         void* tmp;
//         if (pthread_create(&thread, NULL, doprocessing, &newsock) != 0) {
//                     printf("Error while creating new thread\n");
//                     break;
//         }
         pid=pthread_create(&thread, NULL, doprocessing, &newsock);
         if (pid!=0){
             printf("Error while creating new thread\n");
             close(newsock);
             break;
         }
      } /* end of while */
    return 0;

}
