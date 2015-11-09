#include <QCoreApplication>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <string>

int main(int argc, char *argv[])
{
    int sock, newsock, port_num, n;
    char buf[256];
    char command[]="show_users";
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

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

    newsock=accept(sock, (struct sockaddr*)&cli_addr, &clilen);
    if (newsock < 0)
        {
          perror("ERROR on accept");
          exit(1);
        }
    bzero(buf,256);
    n=recv(newsock, buf, 255, 0);
    if (n < 0)
        {
          perror("ERROR reading from socket");
          exit(1);
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
        while (fgets (buf, sizeof(buf), file) != NULL)
                printf("%s", buf);

        printf("\n");
        fclose(file);
    }
    else
        printf("No matches\n");
    n = send(newsock,buf,sizeof(buf), 0);

    if (n < 0)
    {
        perror("ERROR writing to socket");
        exit(1);
    }

        return 0;

}

