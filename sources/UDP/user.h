#ifndef USER
#define USER
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#define bufSize 255
#define maxClients 100
typedef struct{
    char name[bufSize];
    int sock;
    int uid;
    int money;
    struct sockaddr_in cli_addr;
    socklen_t clilen;
}user;

//user();
int set_newid();
int get_money(int usid);
int set_money(int uid, int value);
#endif // USER

