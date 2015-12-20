#ifndef USER
#define USER
#define bufSize 255
#define maxClients 100
typedef struct{
    char name[bufSize];
    int sock;
    int uid;
    int money;
}user;

//user();
int set_newid();
int get_money(int usid);
int set_money(int uid, int value);
#endif // USER

