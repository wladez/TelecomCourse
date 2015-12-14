//client
#include <QCoreApplication>
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define bufSize 255

int authentication(int sockfd);

int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   int aut;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   char quit[]="quit";
   char buffer[256];
   portno=12345;
   if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }

   portno = atoi(argv[2]);

   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }

   server = gethostbyname(argv[1]);
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }

   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);

   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }

   /* Now ask for a message from the user, this message
      * will be read by server
   */
   do {
           aut = authentication(sockfd); //процесс аутентификации клиента
       } while (aut < 0);
   while (1){
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);

    if(strncmp(buffer,quit,sizeof(quit)-1) == 0){
            n = write(sockfd, buffer, strlen(buffer));
            exit(1);
    }
    /* Send message to the server */
    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    /* Now read server response */
    bzero(buffer,256);
    n = read(sockfd, buffer, 255);
    printf("%s\n",buffer);
    if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
    }


  }
    return 0;
}


int authentication(int sockfd){
    char login[bufSize +1];
    char reg[bufSize +1];
    char buffer[bufSize +1];
    char ok[]="ok";
    char sign[]="sign_in";
    char registration[]="register";
    int n;
    int res=-1;
    bzero(buffer, bufSize+1);
    printf("Sign in or register\n");
    scanf("%s", &buffer);
    if(strncmp(buffer,sign,sizeof(sign)-1) == 0){//вход существующего пользователя
        printf("Enter your username:\n");
        scanf("%s", &login);
        n=write(sockfd,"exist",bufSize);//посылка серверу сообщения о том, что входит существующий пользователь
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
        n=write(sockfd,login,bufSize);//посылка серверу имени пользователя
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
        bzero(buffer,bufSize+1);
        n = read(sockfd, buffer, bufSize);//ответ от сервера, правильны ли данные или нет
        if (n < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        if(strncmp(buffer,ok,sizeof(ok)-1) == 0){//данные правильны
            printf("Hello %s, you has successfully logined\n",login);
            res=1;
        }
        else{//данные не правильны
            printf("No such username. Type correct username or register\n");
            res=-1;
        }
    }
    else if(strncmp(buffer,registration,sizeof(registration)-1) == 0){//регистрация нового пользователя
        printf("Create new username:\n");
        scanf("%s", &reg);
        n=write(sockfd,"new",bufSize);//посылка сообщения серверу о регистрации нового пользователя
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
        n=write(sockfd,reg,bufSize);//посылка имени нового пользователя
        if (n < 0) {
            perror("ERROR writing to socket");
            exit(1);
        }
        bzero(buffer,bufSize+1);
        n = read(sockfd, buffer, bufSize);//ответ от сервера, правильны ли данные или нет
        if (n < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        if(strncmp(buffer,ok,sizeof(ok)-1) == 0){//данные правильны
            printf("Hello %s, you has successfully registered and logined\n",reg);
            res=1;
        }
        else{//данные не правильны
            printf("User with this username is already existing. Create other username\n");
        }
     }
    return res;
}
