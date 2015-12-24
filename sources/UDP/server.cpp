#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <cstdlib>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include "server.h"

int clientsCount=0;
user connected[maxClients];

user new_connection(int sockfd){
    struct sockaddr_in clientaddr; /* client addr */
    int clientlen = sizeof(clientaddr); /* byte size of client's address */
    char buf[bufSize];
    int err = 0;

    bzero(buf, bufSize);
    err = recvfrom(sockfd, buf, bufSize, 0, (struct sockaddr *) &clientaddr, (unsigned int*)&clientlen);
    if (err < 0) {
      perror("ERROR reading from socket");
      exit(1);
    }

    int newsockfd;
    uint16_t newport;
    new_socket(&newsockfd, &newport);
    bzero(buf, bufSize);
    sprintf(buf, "%d", newport);
    err = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &clientaddr, clientlen);
    if (err < 0) {
      perror("ERROR writing to socket");
      exit(1);
    }

    // create new socket for this client
    struct sockaddr_in newclientaddr; /* client addr */
    int newclientlen = sizeof(newclientaddr); /* byte size of client's address */
    bzero(buf, bufSize);
    err = recvfrom(newsockfd, buf, bufSize, 0, (struct sockaddr *) &newclientaddr, (unsigned int*)&newclientlen);
    if (err < 0) {
      perror("ERROR reading from socket");
      exit(1);
    }

    user client;
    client.cli_addr=newclientaddr;
    client.clilen=newclientlen;
    client.sock=newsockfd;
    return client;
}

void new_socket(int* sockfd, uint16_t* port){
    struct sockaddr_in serveraddr;
    int err = 0;
    *sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    *port = 12345 + (clientsCount + 1);
    if(*sockfd<0){
        perror("ERROR opening socket");
        exit(1);
    }
    //int optval = 1;
    //setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port =htons((unsigned short)*port);

    err = bind(*sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));
    if(err<0){
        perror("ERROR on binding");
        exit(1);
    }
}

int authentication(user client){
    char buf[bufSize+1];
    char new_client[]="new";
    char exist_client[]="exist";
    char numb[bufSize];
    int n,check;
    int res;
    strcpy(numb,"\t");
    bzero(buf, bufSize+1);
    n=recvfrom(client.sock, buf, bufSize, 0, (struct sockaddr *)&client.cli_addr, &client.clilen);//приём сообщения от клиента, в котором указано
    //будет ли подключен новый пользователь или уже зарегистрированный
    if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
    }
    if(strncmp(buf,exist_client,sizeof(exist_client)-1) == 0){
        //уже существующий пользователь
        bzero(buf,bufSize);
        n=recvfrom(client.sock, buf, bufSize, 0, (struct sockaddr *) &client.cli_addr, &client.clilen);
        if (n < 0) {
          perror("ERROR reading from socket");
          pthread_exit(0);
        }
        check=check_user(buf);//проверка совпадает ли имя которое ввёл пользователь
        //с именем в файле с пользователями
        if (check < 0) {//нет совпадений
            n = sendto(client.sock,"no_match",sizeof(buf), 0, (struct sockaddr *) &client.cli_addr, client.clilen);//говорим клиенту, что нет совпадений
            if (n < 0)
            {
                perror("ERROR writing to socket");
                pthread_exit(0);
            }
            res=-1;
        }
        else{//есть совпадения
            n = sendto(client.sock,"ok",bufSize, 0, (struct sockaddr *) &client.cli_addr, client.clilen);//говорим клиенту, что всё ок
            if (n < 0)
            {
                perror("ERROR writing to socket");
                pthread_exit(0);
            }          
            strcpy(client.name,buf);
            client.uid=check;
            client.money=get_money(client.uid);
            res=1;
            add(client);
            printf("Connected client %s\n",buf);
        }
    }
    else if(strncmp(buf,new_client,sizeof(new_client)-1) == 0){
        //создание нового пользователя
        bzero(buf,bufSize);
        n=recvfrom(client.sock, buf, bufSize, 0, (struct sockaddr *) &client.cli_addr, &client.clilen);
        if (n < 0) {
          perror("ERROR reading from socket");
          pthread_exit(0);
        }

        check=check_user(buf);//проверка нет ли уже такого имени у кого-нибудь
        if(check>0){//есть совпадения
            n = sendto(client.sock,"not_ok",6, 0, (struct sockaddr *) &client.cli_addr, client.clilen);//говорим клиенту, что не ок
            if (n < 0)
            {
                perror("ERROR writing to socket");
                pthread_exit(0);
            }
            res=-1;
        }
        else{//нет совпадений, клиент зарегистрирован
            n = sendto(client.sock,"ok", 2, 0, (struct sockaddr *) &client.cli_addr, client.clilen);//говорим клиенту, что всё ок
            if (n < 0)
            {
                perror("ERROR writing to socket");
                pthread_exit(0);
            }            
            strcpy(client.name,buf);
            client.money=6000;
            printf("Connected client %s\n",buf);
            client.uid=set_newid();//получение id пользователя
            FILE *file;
            char *fname = "/home/user/project_t/us.txt";
            file = fopen(fname,"a");
            fprintf(file,"%s\t",client.name);//запись в файл нового пользователя
            fprintf(file,"%i\n",client.uid);
            fclose(file);
            FILE *mon;
            char *mon_name="/home/user/project_t/money.txt";
            mon=fopen(mon_name,"a");
            fprintf(mon,"%i\t",client.uid);
            fprintf(mon,"%i            \n",client.money);
            fclose(mon);
            add(client);
            res=1;
        }
    }
    return res;
}

void* doprocessing (void* c) {
   user* cli = (user*) c;
   user client = *cli;
   int aut;
   char request[bufSize];
   char buffer[bufSize+1];
   char command[]="show users";
   char quit[]="quit";
   bzero(request,bufSize);
   bzero(buffer,bufSize+1);
   do {
           aut = authentication(client); //процесс аутентификации клиента
       } while (aut < 0);
   while(recvfrom(client.sock, request, bufSize, 0, (struct sockaddr *) &client.cli_addr, &client.clilen)>=0){
       //n=recv(socket, request, bufSize, 0);
//       if(n<0){
//           perror("ERROR reading from socket");
//           break;
//       }
       if(strncmp(request,quit,sizeof(quit)-1) == 0){
           disconnect(client.sock);
           break;
       }
       else if(strncmp(request,command,sizeof(command)-1) == 0){
           show_users(client);
       }
       else if(strcmp(request,"wallet") == 0){
           check_wallet(client);
       }
       else if(strcmp(request,"transf") == 0){
           transfer(client);
       }
       bzero(request,bufSize);
   }
   close(client.sock);
}

void show_users(user client){
    int n;
    char tmp[bufSize];
    char buffer[bufSize+1];
    bzero(buffer,bufSize+1);
    FILE *file;
    char *fname = "/home/user/project_t/us.txt";
    file = fopen(fname,"r");
    if(file == NULL)
    {
        perror("ERROR on openning file with users");
        pthread_exit(0);
    }
    while (fgets (tmp, sizeof(tmp), file) != NULL){
        strncat(buffer,tmp,35);
        printf("%s", tmp);
    }
    printf("\n");
    fclose(file);
    n = sendto(client.sock,buffer,sizeof(buffer), 0, (struct sockaddr *) &client.cli_addr, client.clilen);
    if (n < 0)
    {
        pthread_exit(0);
    }
}

void check_wallet(user client){
    int n,uid;
    char buffer[bufSize+1];
    bzero(buffer,bufSize+1);
    int i = 0;
    int j=0;
    for (i = 0; i <= clientsCount; ++i) {
        if (connected[i].sock == client.sock){
            uid=connected[i].uid;
            connected[i].money=get_money(uid);
            j=i;
        }
    }
    sprintf(buffer,"%i", connected[j].money);
    n = sendto(client.sock,buffer,sizeof(buffer), 0, (struct sockaddr *) &client.cli_addr, client.clilen);
    if (n < 0)
    {
        pthread_exit(0);
    }
}

void transfer(user client){
    int n,value,money;
    int dest=0;
    char buffer[bufSize+1];
    bzero(buffer,bufSize+1);
    n=recvfrom(client.sock, buffer, bufSize+1, 0, (struct sockaddr *) &client.cli_addr, &client.clilen);
    if(n<0){
        perror("ERROR reading from socket");
        pthread_exit(0);
    }
    printf("%s\n",buffer);
    char *tmp=strstr(buffer," ");
    value=atoi(tmp);
    strcpy(tmp,"\0");
    dest=check_user(buffer);
    if (dest < 0) {//нет совпадений
        n = sendto(client.sock,"no_match",8, 0, (struct sockaddr *) &client.cli_addr, client.clilen);//говорим клиенту, что нет совпадений
        if (n < 0)
        {
            pthread_exit(0);
        }
    }
    else{
        money=get_money(dest);
        money+=value;
        set_money(dest,money);
        int i,j;
        for (i = 0; i <= clientsCount; ++i) {
            if (connected[i].sock == client.sock){
                connected[i].money-=value;
                dest=connected[i].uid;
                j=i;
            }
        }
        set_money(dest,connected[j].money);
        n = sendto(client.sock,"ok",2, 0, (struct sockaddr *) &client.cli_addr, client.clilen);
        if (n < 0)
        {
            pthread_exit(0);
        }
    }
}

int check_user(char buf[]){
    char name[bufSize+1];
    char tmp[bufSize+1];
    char id[10];
    int res=-1;
    int k;
    FILE *file;
    char *fname = "/home/user/project_t/us.txt";
    file = fopen(fname,"r");
    bzero(name,bufSize+1);
    strcpy(name,buf);
    if(file == NULL)
    {
        perror("ERROR on openning file with users");
        exit(1);
    }
    int i = 0;
    bzero(tmp,bufSize+1);
    while(fscanf(file,"%s", tmp)!=EOF){
        //fscanf(file,"%s", tmp);
        if(k==0){
            strcpy(id,tmp);
            res=atoi(id);
            printf("%d\n",res);
            break;
        }
        if(!(i%2)){
            k=strcmp(name,tmp);
            printf("%s\n",tmp);
        }
        i++;
    }
    fclose(file);
    return res;
}

void add(user client){
    connected[clientsCount++]=client;
}

void disconnect(int sock){
    int i = 0;
        for (i = 0; i < clientsCount; ++i) {
            if (connected[i].sock == sock)
                break;
        }
        if (i != clientsCount) {
            int n = sendto(connected[i].sock, "exit", 4, 0, (struct sockaddr *)&connected[i].cli_addr, connected[i].clilen);
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            close(sock);
            for (++i; i < clientsCount; ++i) {
                connected[i - 1] = connected[i];
            }
            --clientsCount;
        }
}

void* server_handler(void*){
    while (1) {
            char command[bufSize];
            bzero(command, bufSize);
            scanf("%s", command);
            if (strcmp(command, "show") == 0) {
                for (int i = 0; i < clientsCount; ++i) {
                    printf("%d : %s\n", connected[i].sock, connected[i].name);
                }
            } else if (strcmp(command, "disconnect") == 0) {
                int sock = 0;
                scanf("%d", &sock);
                disconnect(sock);
            } else {
                printf("Undefined command\n");
            }
        }
}
