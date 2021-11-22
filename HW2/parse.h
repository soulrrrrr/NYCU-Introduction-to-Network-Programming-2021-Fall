#ifndef PARSE_H
#define PARSE_H

#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <poll.h>
#include "structs.h"
#define OPEN_MAX 256
using namespace std;

int parse(int sockfd, char *readbuf, ssize_t n, vector<User> &users, vector<string> &login, vector<Board> &boards, vector<Post> &posts);
void parseRegister(int sockfd, vector<User> &users, vector<string> &login, vector<string> &split);
void parseLogin(int sockfd, vector<User> &users, vector<string> &login, vector<string> &split);
void parseLogout(int sockfd, vector<User> &users, vector<string> &login);
int parseExit(int sockfd, vector<string> &login);

void parseCreateBoard(int sockfd, vector<User> &users, vector<string> &login, vector<string> &split, vector<Board> &boards);
void parseCreatePost(int sockfd, vector<User> &users, vector<string> &login, vector<string> &split, vector<Board> &boards, vector<Post> &posts);
void parseListBoard(int sockfd, vector<Board> &boards);
void parseListPost(int sockfd, vector<string> &split, vector<Board> &boards, vector<Post> &posts);
void parseRead(int sockfd, vector<string> &split, vector<Board> &boards, vector<Post> &posts);
void parseDeletePost(int sockfd, vector<string> &login, vector<string> &split, vector<Post> &posts);
void parseUpdatePost(int sockfd, vector<string> &login, vector<string> &split, vector<Post> &posts);
void parseComment(int sockfd, vector<string> &login, vector<string> &split, vector<Post> &posts);


#endif