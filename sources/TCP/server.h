#ifndef SERVER
#define SERVER
void *doprocessing(void *sock);
void show_users(int sock);
int authentication(int sock);
int check_user(char buf[]);
void check_wallet(int sock);
void transfer(int sock);
#endif // SERVER

