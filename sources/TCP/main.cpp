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
#include <map>

using namespace std;

#define bufSize 255

void *doprocessing(void *sock);
int authentication(int sock);
int check_user(char buf[]);

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

    while (1) {
         pthread_t thread;
         newsock = accept(sock, (struct sockaddr *) &cli_addr, &clilen);

         if (newsock < 0) {
            perror("ERROR on accept");
            close(sock);
            exit(1);
         }
         void* tmp;
         if (pthread_create(&thread, NULL, doprocessing, &newsock) != 0) {
                     printf("Error while creating new thread\n");
                     break;
         }

//         /* Create child process */
//         pid = fork();

//         if (pid < 0) {
//            perror("ERROR on fork");
//            exit(1);
//         }

//         if (pid == 0) {
//            /* This is the client process */
//            close(sock);
//            while(true){
//                doprocessing(newsock);
////              exit(0);
//               }
//         }
//         else {
//            close(newsock);
//         }
      } /* end of while */
    return 0;

}



void* doprocessing (void* newsock) {
   int n, socket;
   int aut;
   char buf[256];
   char buffer[256];
   char command[]="show_users";
   char quit[]="quit";
   bzero(buf,256);
   bzero(buffer,256);
   int *tmp=(int*)newsock;
   socket=*tmp;
   do {
           aut = authentication(socket); //процесс аутентификации клиента
       } while (aut < 0);
   while(recv(socket, buf, 255, 0)>=0){
       if(strncmp(buf,quit,sizeof(quit)-1) == 0){
               break;
       }

       if(strncmp(buf,command,sizeof(command)-1) == 0){
           FILE *file;
           char *fname = "/home/user/project_t/us.txt";
           file = fopen(fname,"r");
           if(file == NULL)
               {
                   perror("ERROR on openning file with users");
                   exit(1);
               }
           while (fgets (buf, sizeof(buf), file) != NULL){
                   strncat(buffer,buf,15);
                   printf("%s", buf);
           }
           printf("\n");
           fclose(file);
           n = send(socket,buffer,sizeof(buffer), 0);
       }
       else{
           printf("No matches\n");

           n = send(socket,buf,sizeof(buf), 0);
       }
       if (n < 0)
       {
           perror("ERROR writing to socket");
           exit(1);
       }
    }
}

int authentication(int sock){
    char buf[bufSize+1];
    char new_client[]="new";
    char exist_client[]="exist";
    int n,check;
    int res;
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
            n = send(sock,"ok",sizeof(buf), 0);//говорим клиенту, что всё ок
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            res=1;
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
            n = send(sock,"not_ok",bufSize, 0);//говорим клиенту, что не ок
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            res=-1;
        }
        else{//нет совпадений, клиент зарегистрирован
            n = send(sock,"ok", bufSize, 0);//говорим клиенту, что всё ок
            if (n < 0)
            {
                perror("ERROR writing to socket");
                exit(1);
            }
            res=1;
            printf("Connected client %s\n",buf);

            FILE *file;
            char *fname = "/home/user/project_t/us.txt";
            file = fopen(fname,"a");
            fprintf(file,buf);//запись в файл нового пользователя
            fclose(file);
        }
    }
    return res;
}

int check_user(char buf[]){
    char name[bufSize+1];
    char tmp[bufSize+1];
    char id[10];
    int res=-1;
    int us_id,k;
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
            us_id=atoi(id);
            printf("%d\n",us_id);
            res=1;
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
