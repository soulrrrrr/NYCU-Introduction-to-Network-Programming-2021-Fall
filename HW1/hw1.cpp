#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <string.h>
#include <strings.h>
#include <vector> // std::vector
#include <algorithm> // std::sort
#include <pthread.h>

#define SERV_PORT 9877
#define MAXUSER 1

using namespace std;

struct Mail{
    char* sender;
    char* message;
    Mail(){}
    Mail(char* sender, char* message) {
        this->sender = new char[strlen(sender) + 1];
        strcpy(this->sender, sender);
        this->message = new char[strlen(message) + 1];
        strcpy(this->message, message);
    }
};

struct User{
    char *username;
    char *password;
    vector<Mail> mailbox;
    User() {}
    User(char *username, char *password) {
        this->username = new char[strlen(username) + 1];
        strcpy(this->username, username);
        this->password = new char[strlen(password) + 1];
        strcpy(this->password, password);
    }
};

vector<User> user_list;
char now_login[4096];

int char_to_int(char *c); // Inspired by Thomas Wang
char *int_to_char(int t); // can use atoi instead

int main(int argc, char** argv) {
    int serverfd, clientfd;
    socklen_t clilen;
    sockaddr_in cliaddr, servaddr;

    serverfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(char_to_int(argv[1]));

    if (bind(serverfd, (sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        cerr << "bind() error.\n";
        return 1;
    }

    if (listen(serverfd, MAXUSER) == -1) {
        cerr << "listen() error.\n";
        return 1;
    }

    while(1) {
        
        clientfd = accept(serverfd, (sockaddr *) &cliaddr, &clilen);
        if (clientfd == -1) {
            cerr << "accept() error.\n";
            return 1;
        }
        
        char send_buf[4096];
        char recv_buf[4096];
        char *split[4096];
        strcpy(send_buf, "********************************\n");
        send(clientfd, send_buf, strlen(send_buf), 0);
        strcpy(send_buf, "** Welcome to the BBS server. **\n");
        send(clientfd, send_buf, strlen(send_buf), 0);
        strcpy(send_buf, "********************************\n");
        send(clientfd, send_buf, strlen(send_buf), 0);

        while(1) { // process message from client
            strcpy(send_buf, "% ");
            send(clientfd, send_buf, strlen(send_buf), 0);
            int rec = recv(clientfd, recv_buf, sizeof(recv_buf), 0);
            if (rec < 0) {
                break; // client closed
            }

            recv_buf[rec-1] = '\0';
            char *token;
            int cnt = 0;
            token = strtok(recv_buf, " ");
            while (token != NULL) {
                split[cnt++] = token;
                token = strtok(NULL, " ");
            }
            if (!strcmp(split[0], "register")) {
                if (cnt == 3) { // correct input format
                    bool registered = false;
                    for (auto u : user_list) {
                        if (!strcmp(u.username, split[1])) { // username has been registered
                            registered = true;
                            break;
                        }
                    }
                    if (!registered) {
                        user_list.push_back(User(split[1], split[2]));
                        strcpy(send_buf, "Register successfully.\n");
                        send(clientfd, send_buf, strlen(send_buf), 0);
                    }
                    else {
                        strcpy(send_buf, "Username is already used.\n");
                        send(clientfd, send_buf, strlen(send_buf), 0);
                    }
                    
                }
                else { // wrong input format
                    strcpy(send_buf, "Usage: register <username> <password>\n");
                    send(clientfd, send_buf, strlen(send_buf), 0);
                }
            }
            else if (!strcmp(split[0], "login")) { // login
                if (cnt == 3) {
                    if (strlen(now_login)) { // already login
                        strcpy(send_buf, "Please logout first.\n");
                        send(clientfd, send_buf, strlen(send_buf), 0);
                        continue;
                    }
                    for (auto u : user_list) {
                        if (!strcmp(u.username, split[1])) {
                            if (!strcmp(u.password, split[2])) { // same username and password : login succeed
                                cout << u.username << ": Login successful." << endl;
                                strcpy(now_login, u.username);
                                break;
                            }
                        }
                    }
                    if (strlen(now_login)) {
                        strcpy(send_buf, "Welcome, ");
                        strcat(send_buf, split[1]);
                        strcat(send_buf, ".\n");
                        send(clientfd, send_buf, strlen(send_buf), 0);
                    }
                    else {
                        strcpy(send_buf, "Login failed.\n");
                        send(clientfd, send_buf, strlen(send_buf), 0);
                    }
                }
                else {
                    strcpy(send_buf, "Usage: login <username> <password>\n");
                    send(clientfd, send_buf, strlen(send_buf), 0);
                }
            }
            else if (!strcmp(split[0], "whoami")) { // whoami
                if (strlen(now_login)) {
                    strcpy(send_buf, now_login);
                    strcat(send_buf, "\n");
                    send(clientfd, send_buf, strlen(send_buf), 0);
                }
                else {
                    strcpy(send_buf, "Please login first.\n");
                    send(clientfd, send_buf, strlen(send_buf), 0);
                }
            }
            else if (!strcmp(split[0], "list-user")) { // list-user
                sort(user_list.begin(), user_list.end(), [](const User &a, const User &b){return (strcmp(a.username, b.username)<0);});
                for (auto u : user_list) {
                    strcpy(send_buf, u.username);
                    strcat(send_buf, "\n");
                    send(clientfd, send_buf, strlen(send_buf), 0);
                }
            }
            else if (!strcmp(split[0], "logout")) { // logout
                if (strlen(now_login)) {
                    strcpy(send_buf, "Bye, ");
                    strcat(send_buf, now_login);
                    strcat(send_buf, ".\n");
                    send(clientfd, send_buf, strlen(send_buf), 0);
                    strcpy(now_login, "");
                }
                else {
                    strcpy(send_buf, "Please login first.\n");
                    send(clientfd, send_buf, strlen(send_buf), 0);
                }
            }
            else if (!strcmp(split[0], "send")) { // send
                if (cnt >= 3) { 
                    if (strlen(now_login)) { 
                        bool exists = false;
                        for (auto u = user_list.begin(); u != user_list.end(); u++) {
                            if (!strcmp(u->username, split[1])) {
                                char tmp[4096];
                                char tmpProcessed[4096];
                                strcpy(tmp, split[2]);
                                for(int i = 3; i < cnt; i++) {
                                    strcat(tmp, " ");
                                    strcat(tmp, split[i]);
                                }
                                strncpy(tmpProcessed, &tmp[1], strlen(tmp)-2);
                                u->mailbox.push_back(Mail(now_login, tmpProcessed));
                                cout << now_login << " send a message to " << u->username << "." << endl;
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) { // user didn't exist
                            strcpy(send_buf, "User not existed.\n");
                            send(clientfd, send_buf, strlen(send_buf), 0);
                        }
                    }
                    else { // didn't login
                        strcpy(send_buf, "Please login first.\n");
                        send(clientfd, send_buf, strlen(send_buf), 0);
                    }
                }
                else {
                    strcpy(send_buf, "Usage: send <username> <message>\n");
                    send(clientfd, send_buf, strlen(send_buf), 0);
                }
            }
            else if (!strcmp(split[0], "list-msg")) { // list-msg
                if (strlen(now_login)) {
                    for (auto u = user_list.begin(); u != user_list.end(); u++) {
                        if (!strcmp(u->username, now_login)) {
                            if (u->mailbox.empty()) {
                                strcpy(send_buf, "Your message box is empty.\n");
                                send(clientfd, send_buf, strlen(send_buf), 0);
                            }
                            else {
                                sort(u->mailbox.begin(), u->mailbox.end(), [](const Mail &a, const Mail &b){return (strcmp(a.sender, b.sender)<0);});
                                char tmp[4096];
                                strcpy(tmp, u->mailbox[0].sender);
                                int mailcnt = 0;
                                for (auto m : u->mailbox) {
                                    if (strcmp(m.sender, tmp)) { // count to next user's mail
                                        strcpy(send_buf, int_to_char(mailcnt));
                                        strcat(send_buf, " message from ");
                                        strcat(send_buf, tmp);
                                        strcat(send_buf, ".\n");
                                        send(clientfd, send_buf, strlen(send_buf), 0);
                                        strcpy(tmp, m.sender);
                                        mailcnt = 1;
                                    }
                                    else {
                                        mailcnt++;
                                    }
                                }
                                strcpy(send_buf, int_to_char(mailcnt));
                                strcat(send_buf, " message from ");
                                strcat(send_buf, tmp);
                                strcat(send_buf, ".\n");
                                send(clientfd, send_buf, strlen(send_buf), 0);
                            }
                            break;
                        }     
                    }
                }
                else {
                    strcpy(send_buf, "Please login first.\n");
                    send(clientfd, send_buf, strlen(send_buf), 0);
                }
            }
            else if (!strcmp(split[0], "receive")) { // receive
                if (cnt == 2) {
                    if (strlen(now_login)) {
                        bool exists = false;
                        for(auto u = user_list.begin(); u != user_list.end(); u++) {
                            if (!strcmp(u->username, split[1]))
                                exists = true;
                            if (!strcmp(u->username, now_login)) {
                                bool received = false;
                                for (auto m = u->mailbox.begin(); m != u->mailbox.end(); m++) {
                                    if (!strcmp(m->sender, split[1])) {
                                        strcpy(send_buf, m->message);
                                        strcat(send_buf, "\n");
                                        send(clientfd, send_buf, strlen(send_buf), 0);
                                        u->mailbox.erase(m);
                                        received = true;
                                        break;
                                    }
                                }
                                // if (!received) {
                                //     strcpy(send_buf, "No message from this user.\n");
                                //     send(clientfd, send_buf, strlen(send_buf), 0);
                                // }
                                break; 
                            }
                        }
                        if (!exists) {
                            strcpy(send_buf, "User not existed.\n");
                            send(clientfd, send_buf, strlen(send_buf), 0);
                        }
                    }
                    else {
                        strcpy(send_buf, "Please login first.\n");
                        send(clientfd, send_buf, strlen(send_buf), 0);
                    }
                }
                else {
                    strcpy(send_buf, "Usage: receive <username>\n");
                    send(clientfd, send_buf, strlen(send_buf), 0);
                }
            }
            else if (!strcmp(split[0], "exit")) {
                if (strlen(now_login)) {
                    strcpy(send_buf, "Bye, ");
                    strcat(send_buf, now_login);
                    strcat(send_buf, ".\n");
                    send(clientfd, send_buf, strlen(send_buf), 0);
                    strcpy(now_login, "");
                }
                break;
            }
            // else {
            //     strcpy(send_buf, "GG.\n");
            //     send(clientfd, send_buf, strlen(send_buf), 0);
            // }
            
        }

        // if ( (childpid = Fork()) == 0) { /* child process */
        //     Close(listenfd); /* close listening socket */
        //     str_echo(connfd); /* process the request */
        //     exit(0);
        // }
        close(clientfd);
    }
    cout << "Server closed." << endl;
    return 0;
}

int char_to_int(char *c) {
    int t = 0;
    for (int i = 0; i < strlen(c); i++) {
        t *= 10;
        t += c[i] - '0';
    }
    return t;
}

char *int_to_char(int t) {
    string s = to_string(t);
    char *c = new char[s.length()+1];
    strcpy(c, s.c_str());
    return c;
}

