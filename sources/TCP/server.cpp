#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include "server.h"
#include "user.h"

int clientsCount=0;
user connected[maxClients];

int authentication(int sock){
    //user connected;
    char buf[bufSize+1];
    char new_client[]="new";
    char exist_client[]="exist";
    char numb[bufSize];
    int n,check;
    int res;
    strcpy(numb,"\t");
    bzero(buf, bufSize+1);
    n=recv(sock, buf, bufSize, 0);//приём сообщения от клиента, в котором указано
    //будет ли подключен новый пользователь или уже зарегистрированный
    if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
    }
    if(strncmp(buf,exist_client,sizeof(exist_client)-1) == 0){
        //уже существующий пользователь
        bzero(buf,bufSize);
        n=recv(sock, buf, bufSize, 0);
        if (n < 0) {
          perror("ERROR reading from socket");
          exit(1);
        }
        check=check_user(buf);//проверка совпадает ли имя которое ввёл пользователь
        //с именем в файле с пользователями
        if (check < 0) {//нет совпадений
            n = send(sock,"no_match",sizeof(buf), 0);//говорим клиенту, что нет совпадений
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            res=-1;
        }
        else{//есть совпадения
            n = send(sock,"ok",bufSize, 0);//говорим клиенту, что всё ок
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            connected[clientsCount].sock=sock;
            strcpy(connected[clientsCount].name,buf);
            connected[clientsCount].uid=check;
            connected[clientsCount].money=get_money(connected[clientsCount].uid);
            res=1;
            clientsCount++;
            printf("Connected client %s\n",buf);
        }
    }
    else if(strncmp(buf,new_client,sizeof(new_client)-1) == 0){
        //создание нового пользователя
        bzero(buf,bufSize);
        n=recv(sock, buf, bufSize, 0);
        if (n < 0) {
          perror("ERROR reading from socket");
          exit(1);
        }

        check=check_user(buf);//проверка нет ли уже такого имени у кого-нибудь
        if(check>0){//есть совпадения
            n = send(sock,"not_ok",6, 0);//говорим клиенту, что не ок
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            res=-1;
        }
        else{//нет совпадений, клиент зарегистрирован
            n = send(sock,"ok", 2, 0);//говорим клиенту, что всё ок
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            connected[clientsCount].sock=sock;
            strcpy(connected[clientsCount].name,buf);
            connected[clientsCount].money=5000;
            printf("Connected client %s\n",buf);
            connected[clientsCount].uid=set_newid();//получение id пользователя
            FILE *file;
            char *fname = "/home/user/project_t/us.txt";
            file = fopen(fname,"a");
            fprintf(file,"%s\t",connected[clientsCount].name);//запись в файл нового пользователя
            fprintf(file,"%i\n",connected[clientsCount].uid);
            fclose(file);
            FILE *mon;
            char *mon_name="/home/user/project_t/money.txt";
            mon=fopen(mon_name,"a");
            fprintf(mon,"%i\t",connected[clientsCount].uid);
            fprintf(mon,"%i      \n",connected[clientsCount].money);
            fclose(mon);
            clientsCount++;
            res=1;
        }
    }
    return res;
}

void* doprocessing (void* newsock) {
   int n, socket;
   int aut;
   char request[bufSize];
   char buffer[bufSize+1];
   char command[]="show users";
   char quit[]="quit";
   bzero(request,bufSize);
   bzero(buffer,bufSize+1);
   int *tmp=(int*)newsock;
   socket=*tmp;
   do {
           aut = authentication(socket); //процесс аутентификации клиента
       } while (aut < 0);
   while(1){
       n=recv(socket, request, bufSize, 0);
       if(n<0){
           perror("ERROR reading from socket");
           break;
       }
       if(strncmp(request,quit,sizeof(quit)-1) == 0){
               break;
       }
       else if(strncmp(request,command,sizeof(command)-1) == 0){
           show_users(socket);
       }
       else if(strcmp(request,"wallet") == 0){
           check_wallet(socket);
       }
       else if(strcmp(request,"transf") == 0){
           transfer(socket);
       }
       bzero(request,bufSize);
   }
   close(socket);
}

void show_users(int sock){
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
            exit(1);
        }
    while (fgets (tmp, sizeof(tmp), file) != NULL){
            strncat(buffer,tmp,35);
            printf("%s", tmp);
    }
    printf("\n");
    fclose(file);
    n = send(sock,buffer,sizeof(buffer), 0);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }
}

void check_wallet(int sock){
    int n,uid;
    char buffer[bufSize+1];
    bzero(buffer,bufSize+1);
    int i = 0;
    int j=0;
    for (i = 0; i <= clientsCount; ++i) {
        if (connected[i].sock == sock){
            uid=connected[i].uid;
            connected[i].money=get_money(uid);
            j=i;
        }
    }
    sprintf(buffer,"%i", connected[j].money);
    n = send(sock,buffer,sizeof(buffer), 0);
    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }
}

void transfer(int sock){
    int n,value,money;
    int dest=0;
    char buffer[bufSize+1];
    bzero(buffer,bufSize+1);
    n=recv(sock, buffer, bufSize+1, 0);
    if(n<0){
        perror("ERROR reading from socket");
        exit(1);
    }
    printf("%s\n",buffer);
    char *tmp=strstr(buffer," ");
    value=atoi(tmp);
    strcpy(tmp,"\0");
    dest=check_user(buffer);
    if (dest < 0) {//нет совпадений
        n = send(sock,"no_match",8, 0);//говорим клиенту, что нет совпадений
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(1);
        }
    }
    else{
        money=get_money(dest);
        money+=value;
        set_money(dest,money);
        int i,j;
        for (i = 0; i <= clientsCount; ++i) {
            if (connected[i].sock == sock){
                connected[i].money-=value;
                dest=connected[i].uid;
                j=i;
            }
        }
        set_money(dest,connected[j].money);
        n = send(sock,"ok",2, 0);
        if (n < 0)
        {
            perror("ERROR writing to socket");
            exit(1);
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
