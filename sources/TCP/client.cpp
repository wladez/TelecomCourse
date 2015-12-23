//client
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define bufSize 255

int authentication(int sockfd);
void showUsers(int sockfd, char command[]);
void checkWallet(int sockfd);
void transfer(int sockfd);
int disconnect(int sockfd, char* buf);

int main(int argc, char *argv[]) {
	WORD wVersionRequested = MAKEWORD(1, 1);       // Stuff for WSA functions
	WSADATA wsaData;
   int sockfd, portno, n;
   int aut;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   char quit[]="quit";
   char show[]="show users";
   char wallet[]="check wallet";
   char transf[]="transfer";
   char buffer[bufSize+1];

   WSAStartup(wVersionRequested, &wsaData);
   //portno=12345;
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

   memset((char *) &serv_addr, 0, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
   //strncpy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
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
    printf("Enter the command: ");
    memset(buffer, 0, bufSize+1);
    fgets(buffer,bufSize+1,stdin);

    if(strncmp(buffer,quit,sizeof(quit)-1) == 0){
            n = send(sockfd, buffer, strlen(buffer),0);
            if (n < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
            closesocket(sockfd);
            break;
    }
    else if(strncmp(buffer,show,sizeof(show)-1) == 0){
        showUsers(sockfd,buffer);
    }
    else if(strncmp(buffer, wallet,sizeof(wallet)-1) == 0){
        checkWallet(sockfd);
    }
    else if(strncmp(buffer, transf,sizeof(transf)-1) == 0){
        transfer(sockfd);
    }
    else{
        printf("Undefined command\n");
    }
  }
    return 0;
}

void showUsers(int sockfd, char command[]){
    int n;
    char buffer[bufSize+1];
    memset(buffer, 0, bufSize);
    strcpy(buffer,command);
    n = send(sockfd, buffer, strlen(buffer),0);
    if (n < 0) {
        perror("ERROR writing to socket");
        closesocket(sockfd);
        exit(1);
    }

    /* Now read server response */
    memset(buffer, 0, bufSize);
    n = recv(sockfd, buffer, bufSize+1,0);    
    if (n < 0) {
      perror("ERROR reading from socket");
      closesocket(sockfd);
      exit(1);
    }
    disconnect(sockfd, buffer);
    printf("%s\n",buffer);
}

void checkWallet(int sockfd){
    int n;
    char buffer[bufSize+1];
    n=send(sockfd, "wallet", 6, 0);
    if (n < 0) {
        perror("ERROR writing to socket");
        closesocket(sockfd);
        exit(1);
    }
	memset(buffer, 0, bufSize+1);
    n = recv(sockfd, buffer, bufSize+1,0);    
    if (n < 0) {
      perror("ERROR reading from socket");
      closesocket(sockfd);
      exit(1);
    }
    disconnect(sockfd, buffer);
    printf("%s\n",buffer);
}

void transfer(int sockfd){
    int n;
    char tmp[bufSize];
    char buffer[bufSize+1];
    memset(buffer, 0, bufSize+1);
    n=send(sockfd, "transf", 6, 0);
    if (n < 0) {
        perror("ERROR writing to socket");
        closesocket(sockfd);
        exit(1);
    }
    printf("To who and how much do you want transfer money?\n");
    scanf("%s",buffer);
    scanf("%s",tmp);
    strcat(buffer," ");
    strcat(buffer,tmp);
    n=send(sockfd, buffer, bufSize+1, 0);
    memset(buffer, 0, bufSize+1);
    n=recv(sockfd, buffer, bufSize+1,0);
    if (n < 0) {
      perror("ERROR reading from socket");
      closesocket(sockfd);
      exit(1);
    }
    disconnect(sockfd, buffer);
    if(strcmp(buffer,"no_match")==0){
        printf("There is no user with such username\n");
    }
    else if(strcmp(buffer,"ok")==0){
        printf("Operation done\n");
    }
    else{
        printf("Some error occurs during the operation\n");
    }
}

int disconnect(int sockfd, char* buf){
    if (strcmp(buf, "exit") == 0) {
            printf("Disconnected from server\n");
            closesocket(sockfd);
            exit(1);
        }
        else return -1;
}

int authentication(int sockfd){
    char login[bufSize +1];
    char reg[bufSize +1];
    char buffer[bufSize +1];
    char ok[]="ok";
    char sign[]="sign in";
    char registration[]="register";
    int n;
    int res=-1;
    memset(buffer, 0, bufSize+1);
    printf("Sign in or register\n");
    fgets(buffer,bufSize+1,stdin);
    //scanf("%s", &buffer);
    if(strncmp(buffer,sign,sizeof(sign)-1) == 0){//вход существующего пользователя
        printf("Enter your username:\n");
        scanf("%s", &login);
        n=send(sockfd,"exist",bufSize,0);//посылка серверу сообщения о том, что входит существующий пользователь
        if (n < 0) {
            perror("ERROR writing to socket");
            closesocket(sockfd);
            exit(1);
        }
        n=send(sockfd,login,bufSize,0);//посылка серверу имени пользователя
        if (n < 0) {
            perror("ERROR writing to socket");
            closesocket(sockfd);
            exit(1);
        }
        memset(buffer, 0, bufSize+1);
        n = recv(sockfd, buffer, bufSize+1,0);//ответ от сервера, правильны ли данные или нет
        if (n < 0) {
            perror("ERROR reading from socket");
            closesocket(sockfd);
            exit(1);
        }
        disconnect(sockfd, buffer);
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
        n=send(sockfd,"new",bufSize,0);//посылка сообщения серверу о регистрации нового пользователя
        if (n < 0) {
            perror("ERROR writing to socket");
            closesocket(sockfd);
            exit(1);
        }
        n=send(sockfd,reg,bufSize,0);//посылка имени нового пользователя
        if (n < 0) {
            perror("ERROR writing to socket");
            closesocket(sockfd);
            exit(1);
        }
        memset(buffer, 0, bufSize+1);
        n = recv(sockfd, buffer, bufSize+1,0);//ответ от сервера, правильны ли данные или нет
        if (n < 0) {
            perror("ERROR reading from socket");
            closesocket(sockfd);
            exit(1);
        }
        disconnect(sockfd, buffer);
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
