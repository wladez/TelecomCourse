#ifndef SERVER
#define SERVER
#include "user.h"
void *doprocessing(void *c);
void show_users(user client);
int authentication(user client);
int check_user(char buf[]);
void check_wallet(user client);
void transfer(user client);
void *server_handler(void *);
void disconnect(int sock);
void add(user client);
user new_connection(int sockfd);
void new_socket(int* sockfd, uint16_t* port);
#endif // SERVER

