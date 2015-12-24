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

typedef struct {
	int sockfd;
	struct sockaddr_in serv_addr;
	int clilen;
} Uclient;

int authentication(Uclient client);
void showUsers(char command[], Uclient client);
void checkWallet(Uclient client);
void transfer(Uclient client);
int disconnect(int sockfd, char* buf);

int main(int argc, char *argv[]) {
	WORD wVersionRequested = MAKEWORD(2, 2);       // Stuff for WSA functions
	WSADATA wsaData;
   int sock, portno, n=0;
   int aut;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   char quit[]="quit";
   char show[]="show users";
   char wallet[]="check wallet";
   char transf[]="transfer";
   char buffer[bufSize+1];
   char buf[bufSize];

   WSAStartup(wVersionRequested, &wsaData);
   //portno=12345;
   if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }

   portno = atoi(argv[2]);

   /* Create a socket point */
   sock = socket(AF_INET, SOCK_DGRAM, 0);

   if (sock == SOCKET_ERROR) {
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
   int len = sizeof(serv_addr);


	n = sendto(sock, "client", strlen("client"), 0, (struct sockaddr *) &serv_addr, len);
	if (n < 0) {
        perror("ERROR writing to socket");
        closesocket(sock);
        exit(1);
    }

	//recieve new port
	memset(buf, 0, bufSize);
	n = recvfrom(sock, buf, bufSize, 0, (struct sockaddr *) &serv_addr, &len);
	if (n < 0) {
      perror("ERROR reading from socket");
      closesocket(sock);
      exit(1);
    } 
	disconnect(sock, buf);
	//close old socket
	closesocket(sock);
	WSACleanup();

	int newport = atoi(buf);

	WSADATA wsa2;
	if (WSAStartup(MAKEWORD(2, 2), &wsa2) != 0) {
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in new_addr;
	int sockfd, new_slen = sizeof(new_addr);

	//create socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR) {
		exit(EXIT_FAILURE);
	}

	//setup address structure
	memset((char *)&new_addr, 0, new_slen);
	new_addr.sin_family = AF_INET;
	new_addr.sin_port = htons(newport);
	new_addr.sin_addr.S_un.S_addr = inet_addr(argv[1]);
	
	n = sendto(sockfd, "newclient", strlen("newclient"), 0, (struct sockaddr *) &new_addr, new_slen);
	if (n < 0) {
      perror("ERROR reading from socket");
      closesocket(sockfd);
      exit(1);
    } 

	Uclient client;
	client.sockfd = sockfd;
	client.serv_addr = new_addr;
	client.clilen = sizeof(new_addr);

   do {
           aut = authentication(client); //процесс аутентификации клиента
       } while (aut < 0);
   while (1){
    printf("Enter the command: ");
    memset(buffer, 0, bufSize+1);
    fgets(buffer,bufSize+1,stdin);

    if(strncmp(buffer,quit,sizeof(quit)-1) == 0){
            n = sendto(client.sockfd, buffer, strlen(buffer),0, (struct sockaddr*)&client.serv_addr, client.clilen);
            if (n < 0) {
                perror("ERROR writing to socket");
                exit(1);
            }
            closesocket(client.sockfd);
            break;
    }
    else if(strncmp(buffer,show,sizeof(show)-1) == 0){
        showUsers(buffer, client);
    }
    else if(strncmp(buffer, wallet,sizeof(wallet)-1) == 0){
        checkWallet(client);
    }
    else if(strncmp(buffer, transf,sizeof(transf)-1) == 0){
        transfer(client);
    }
    else{
        printf("Undefined command\n");
    }
  }
    return 0;
}

void showUsers(char command[], Uclient client){
    int n;
    char buffer[bufSize+1];
    memset(buffer, 0, bufSize);
    strcpy(buffer,command);
	n = sendto(client.sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&client.serv_addr, client.clilen);
    if (n < 0) {
        perror("ERROR writing to socket");
        closesocket(client.sockfd);
        exit(1);
    }

    /* Now read server response */
    memset(buffer, 0, bufSize);
	n = recvfrom(client.sockfd, buffer, bufSize+1, 0, (struct sockaddr*)&client.serv_addr, &client.clilen);    
    if (n < 0) {
      perror("ERROR reading from socket");
      closesocket(client.sockfd);
      exit(1);
    }
    disconnect(client.sockfd, buffer);
    printf("%s\n",buffer);
}

void checkWallet(Uclient client){
    int n;
    char buffer[bufSize+1];
    n=sendto(client.sockfd, "wallet", 6, 0, (struct sockaddr*)&client.serv_addr, client.clilen);
    if (n < 0) {
        perror("ERROR writing to socket");
        closesocket(client.sockfd);
        exit(1);
    }
	memset(buffer, 0, bufSize+1);
    n = recvfrom(client.sockfd, buffer, bufSize+1, 0, (struct sockaddr*)&client.serv_addr, &client.clilen);    
    if (n < 0) {
      perror("ERROR reading from socket");
      closesocket(client.sockfd);
      exit(1);
    }
    disconnect(client.sockfd, buffer);
    printf("%s\n",buffer);
}

void transfer(Uclient client){
    int n;
    char tmp[bufSize];
    char buffer[bufSize+1];
    memset(buffer, 0, bufSize+1);
    n=sendto(client.sockfd, "transf", 6, 0, (struct sockaddr*)&client.serv_addr, client.clilen);
    if (n < 0) {
        perror("ERROR writing to socket");
        closesocket(client.sockfd);
        exit(1);
    }
    printf("To who and how much do you want transfer money?\n");
    scanf("%s",buffer);
    scanf("%s",tmp);
    strcat(buffer," ");
    strcat(buffer,tmp);
    n=sendto(client.sockfd, buffer, bufSize+1, 0, (struct sockaddr*)&client.serv_addr, client.clilen);
    memset(buffer, 0, bufSize+1);
    n=recvfrom(client.sockfd, buffer, bufSize+1, 0, (struct sockaddr*)&client.serv_addr, &client.clilen);
    if (n < 0) {
      perror("ERROR reading from socket");
      closesocket(client.sockfd);
      exit(1);
    }
    disconnect(client.sockfd, buffer);
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

int authentication(Uclient client){
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
        n=sendto(client.sockfd,"exist",bufSize, 0, (struct sockaddr*)&client.serv_addr, client.clilen);//посылка серверу сообщения о том, что входит существующий пользователь
        if (n < 0) {
            perror("ERROR writing to socket");
            closesocket(client.sockfd);
            exit(1);
        }
        n=sendto(client.sockfd,login,bufSize, 0, (struct sockaddr*)&client.serv_addr, client.clilen);//посылка серверу имени пользователя
        if (n < 0) {
            perror("ERROR writing to socket");
            closesocket(client.sockfd);
            exit(1);
        }
        memset(buffer, 0, bufSize+1);
        n = recvfrom(client.sockfd, buffer, bufSize+1, 0, (struct sockaddr*)&client.serv_addr, &client.clilen);//ответ от сервера, правильны ли данные или нет
        if (n < 0) {
            perror("ERROR reading from socket");
            closesocket(client.sockfd);
            exit(1);
        }
        disconnect(client.sockfd, buffer);
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
        n=sendto(client.sockfd,"new",bufSize, 0, (struct sockaddr*)&client.serv_addr, client.clilen);//посылка сообщения серверу о регистрации нового пользователя
        if (n < 0) {
            perror("ERROR writing to socket");
            closesocket(client.sockfd);
            exit(1);
        }
        n=sendto(client.sockfd,reg,bufSize, 0, (struct sockaddr*)&client.serv_addr, client.clilen);//посылка имени нового пользователя
        if (n < 0) {
            perror("ERROR writing to socket");
            closesocket(client.sockfd);
            exit(1);
        }
        memset(buffer, 0, bufSize+1);
        n = recvfrom(client.sockfd, buffer, bufSize+1, 0, (struct sockaddr*)&client.serv_addr, &client.clilen);//ответ от сервера, правильны ли данные или нет
        if (n < 0) {
            perror("ERROR reading from socket");
            closesocket(client.sockfd);
            exit(1);
        }
        disconnect(client.sockfd, buffer);
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
