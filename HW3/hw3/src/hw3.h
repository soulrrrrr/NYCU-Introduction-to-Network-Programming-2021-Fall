#ifndef HW3_H
#define HW3_H

#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <poll.h>
#include <string>
#include <cstring>
#include "structs.h"

#define OPEN_MAX 256
using namespace std;

int parse(int sockfd, char *readbuf, ssize_t n, vector<User> &users, vector<FDdata> &login);
void parseRegister(int sockfd, vector<User> &users, vector<FDdata> &login, vector<string> &split);
void parseLogin(int sockfd, vector<User> &users, vector<FDdata> &login, vector<string> &split);
void parseLogout(int sockfd, vector<User> &users, vector<FDdata> &login, vector<string> &split);
int parseExit(int sockfd, vector<FDdata> &login, vector<string> &split);
void parseEnterChatRoom(int sockfd, vector<User> &users, vector<FDdata> &login, vector<string> &split);



bool isNumber(const string& str);
int char_to_int(char *c);

#endif