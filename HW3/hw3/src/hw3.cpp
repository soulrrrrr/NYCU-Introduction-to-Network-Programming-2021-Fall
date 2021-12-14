#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <vector> // std::vector
#include <algorithm> // std::sort
#include <map> //std::map
#include <poll.h>
#include <sstream>      // std::stringstream
#include <ctime>        // date
#include "hw3.h"

#define LISTENQ 10
#define MAXLINE 4096
#define INFTIM -1
using namespace std;

vector<User> users;
map<string, int> blacklist;
vector<FDdata> login(OPEN_MAX);
string chatHistory;
vector<char*> filterWords={"how","you","or","pek0","tea","ha","kon","pain","Starburst Stream"};
pollfd		client[OPEN_MAX];
int maxi;

void filter(char *name, char *msg);
string base64Encode(string in);
string base64Decode(string in);

int main(int argc, char** argv) {
    int i, tcpfd, udpfd, clientfd, sockfd;
    int	nready;
    ssize_t	n; /* read buffer size */
    fd_set rset, allset;
    char buf[MAXLINE];
    string msg;
    socklen_t clilen;
    //pollfd		client[OPEN_MAX];
    sockaddr_in cliaddr, servaddr;
        /* 4create TCP socket */
    tcpfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(char_to_int(argv[1]));
    const int on = 1;
    setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if (bind(tcpfd, (sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        cerr << "bind(tcp) error.\n";
        return 1;
    }

    if (listen(tcpfd, LISTENQ) == -1) {
        cerr << "listen(tcp) error.\n";
        return 1;
    }

        /* 4create UDP socket */
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(char_to_int(argv[1]));

    if (bind(udpfd, (sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        cerr << "bind(udp) error.\n";
        return 1;
    }

    client[0].fd = tcpfd;
	client[0].events = POLLRDNORM;
    client[1].fd = udpfd;
	client[1].events = POLLRDNORM;

	for (i = 2; i < OPEN_MAX; i++)
		client[i].fd = -1;		/* -1 indicates available entry */
	maxi = 1;					/* max index into client[] array */
    

    while(1) {
        nready = poll(client, maxi+1, INFTIM);
        
        if (client[0].revents & POLLRDNORM) {	/* new TCP client connection */
			clilen = sizeof(cliaddr);
			clientfd = accept(tcpfd, (sockaddr *) &cliaddr, &clilen);
            if (clientfd == -1) {
                cerr << "accept() error.\n";
                return 1;
            }
            else {
                cout << "file descriptor " << clientfd << " has new user." << endl;
                msg = "********************************\n";
                //write(clientfd, msg.c_str(), msg.length());
                msg += "** Welcome to the BBS server. **\n";
                //write(clientfd, msg.c_str(), msg.length());
                msg += "********************************\n";
                //write(clientfd, msg.c_str(), msg.length());
                msg += "% ";
                write(clientfd, msg.c_str(), msg.length());
            }

            for (i = 2; i < OPEN_MAX; i++)
                if (client[i].fd < 0) {
                    client[i].fd = clientfd;	/* save descriptor */
                    login[clientfd].udpaddr = cliaddr;
                    break;
                }

            if (i == OPEN_MAX) {
                cerr << "too many clients\n";
                return 1;
            }

            client[i].events = POLLRDNORM;
            if (i > maxi)
                maxi = i;				/* max index in client[] array */

            if (--nready <= 0)
                continue;				/* no more readable descriptors */

            // users.push_back(User("a", "a"));
            // login[clientfd] = "a";
            // boards.push_back(Board(1, "hi", "a"));
        }
        
        if (client[1].revents & POLLRDNORM) {	/* UDP client connection */
			clilen = sizeof(cliaddr);
            memset(buf, 0, sizeof(buf));
			n = recvfrom(udpfd, buf, MAXLINE, 0, (sockaddr *) &cliaddr, &clilen);
            buf[n]=0;
            uint8_t ver;
            memcpy(&ver, buf+1, 1);
            unsigned char sendBuf[MAXLINE];
            memset(sendBuf, 0, sizeof(sendBuf));
            uint16_t name_len;
            uint16_t msg_len;
            char name[MAXLINE];
            memset(name, 0, sizeof(name));
            char msg[MAXLINE];
            memset(msg, 0, sizeof(msg));
            if (ver == 1) {
                
                //uint16_t name_len;
                memcpy(&name_len, buf+2, 2);

                //char name[MAXLINE];
                //memset(name, 0, sizeof(name));
                memcpy(&name, buf+4, sizeof(name));
                name[name_len]=0;

                //uint16_t msg_len;
                memcpy(&msg_len, buf+4+strlen(name), 2);

                //char msg[MAXLINE];
                //memset(msg, 0, sizeof(msg));
                memcpy(&msg, buf+4+strlen(name)+2, sizeof(msg));
                msg[msg_len]=0;

                filter(name, msg);
                chatHistory += name;
                chatHistory += ":";
                chatHistory += msg;
                chatHistory += "\n";
                //char sendBuf[MAXLINE];
                // memset(sendBuf, 0, sizeof(sendBuf));
                // strcat(sendBuf, name);
                // strcat(sendBuf, ":");
                // strcat(sendBuf, msg);
                // strcat(sendBuf, "\n% ");
            }
            else {
                // cout << buf << endl;
                // string recvb(buf);
                // cout << recvb << endl;
                // // string recvb_decode = base64_decode(recvb);
                // // cout << recvb_decode << endl;
                int sep1 = 0, sep2 = 0;
                for(int i = 2; i < n; i++) {
                    if (buf[i] == '\n') {
                        if (sep1 == 0)
                            sep1 = i;
                        else
                            sep2 = i;
                    }
                }
                cout << sep1 << " " << sep2 << endl;
                char namet[MAXLINE];
                memset(namet, 0, sizeof(namet));
                char msgt[MAXLINE];
                memset(msgt, 0, sizeof(msgt));
                strncpy(namet, buf+2, sep1-2);
                strncpy(msgt, buf+sep1+1, sep2-sep1-1);
                string namets(namet);
                string msgts(msgt);
                string name2 = base64Decode(namets);
                string msg2 = base64Decode(msgts);
                char name2t[MAXLINE];
                memset(name2t, 0, sizeof(name2t));
                char msg2t[MAXLINE];
                memset(msg2t, 0, sizeof(msg2t));
                strcpy(name2t, name2.c_str());
                strcpy(msg2t, msg2.c_str());

                filter(name2t, msg2t);
                chatHistory += name2t;
                chatHistory += ":";
                chatHistory += msg2t;
                chatHistory += "\n";
                
                strcpy(name, name2t);
                strcpy(msg, msg2t);
            }
            
            
            for (i = 2; i < OPEN_MAX; i++) {
                if (client[i].fd > 3 && login[client[i].fd].inChatRoom) {
                    if (strcmp((login[client[i].fd].username).c_str(), name) == 0) {
                        for(auto u : users) {
                            if (login[client[i].fd].username == u.username) {
                                if (u.black >= 3) {
                                    blacklist[login[client[i].fd].username] = 1;
                                    string msgs;
                                    msgs += "Bye, ";
                                    msgs += login[client[i].fd].username;
                                    msgs += ".\n% ";
                                    write(client[i].fd, msgs.c_str(), msgs.length());
                                    login[client[i].fd].username = "";
                                    login[client[i].fd].inChatRoom = false;
                                }
                                else {
                                    cliaddr.sin_port = login[client[i].fd].udpaddr.sin_port;
                                    if (login[client[i].fd].version == 1) {
                                        sendBuf[0] = 1;
                                        sendBuf[1] = 1;
                                        //cout << name_len << endl;
                                        uint16_t namelen = (uint16_t)htons(strlen(name));
                                        uint16_t msglen = (uint16_t)htons(strlen(msg));
                                        //cout << namelen << " " << msglen << endl;
                                        memcpy(sendBuf+2, &namelen, 2);
                                        //cout << (uint)sendBuf[2] << " " << (uint)sendBuf[3] << endl;
                                        memcpy(sendBuf+4, name, strlen(name));
                                        memcpy(sendBuf+4+strlen(name), &msglen, 2);
                                        memcpy(sendBuf+4+strlen(name)+2, msg, strlen(msg));
                                        sendto(udpfd, sendBuf, 6+strlen(name)+strlen(msg), 0, (sockaddr *) &(cliaddr), clilen);
                                    }
                                    else {
                                        sendBuf[0] = 1;
                                        sendBuf[1] = 2;
                                        string nameEnc(name);
                                        string msgEnc(msg);
                                        string nameEncoded = base64Encode(nameEnc);
                                        string msgEncoded = base64Encode(msgEnc);
                                        memcpy(sendBuf+2, nameEncoded.c_str(), nameEncoded.length());
                                        sendBuf[2+nameEncoded.length()] = '\n';
                                        memcpy(sendBuf+2+nameEncoded.length()+1, msgEncoded.c_str(), msgEncoded.length());
                                        sendBuf[2+nameEncoded.length()+1+msgEncoded.length()] = '\n';
                                        sendto(udpfd, sendBuf, 4+nameEncoded.length()+msgEncoded.length(), 0, (sockaddr *) &(cliaddr), clilen);
                                    }
                                }
                                break;
                            }
                        }
                    }
                    else {
                        cliaddr.sin_port = login[client[i].fd].udpaddr.sin_port;
                        if (login[client[i].fd].version == 1) {
                            sendBuf[0] = 1;
                            sendBuf[1] = 1;
                            //cout << name_len << endl;
                            uint16_t namelen = (uint16_t)htons(strlen(name));
                            uint16_t msglen = (uint16_t)htons(strlen(msg));
                            //cout << namelen << " " << msglen << endl;
                            memcpy(sendBuf+2, &namelen, 2);
                            //cout << (uint)sendBuf[2] << " " << (uint)sendBuf[3] << endl;
                            memcpy(sendBuf+4, name, strlen(name));
                            memcpy(sendBuf+4+strlen(name), &msglen, 2);
                            memcpy(sendBuf+4+strlen(name)+2, msg, strlen(msg));
                            sendto(udpfd, sendBuf, 6+strlen(name)+strlen(msg), 0, (sockaddr *) &(cliaddr), clilen);
                        }
                        else {
                            sendBuf[0] = 1;
                            sendBuf[1] = 2;
                            string nameEnc(name);
                            string msgEnc(msg);
                            string nameEncoded = base64Encode(nameEnc);
                            string msgEncoded = base64Encode(msgEnc);
                            memcpy(sendBuf+2, nameEncoded.c_str(), nameEncoded.length());
                            sendBuf[2+nameEncoded.length()] = '\n';
                            memcpy(sendBuf+2+nameEncoded.length()+1, msgEncoded.c_str(), msgEncoded.length());
                            sendBuf[2+nameEncoded.length()+1+msgEncoded.length()] = '\n';
                            sendto(udpfd, sendBuf, 4+nameEncoded.length()+msgEncoded.length(), 0, (sockaddr *) &(cliaddr), clilen);
                        }
                    }
                        
                }
            }

            if (--nready <= 0)
                continue;				/* no more readable descriptors */
        }

        for (i = 2; i <= maxi; i++) {	/* check all clients for data */
            if ( (sockfd = client[i].fd) < 0)
                continue;
            if (client[i].revents & (POLLRDNORM | POLLERR)) {
                if ( (n = read(sockfd, buf, MAXLINE)) < 0) {
                    if (errno == ECONNRESET) {
                            /*4connection reset by client */
                        close(sockfd);
                        client[i].fd = -1;
                    }
                    else {
                        cerr << "read error.\n";
                        return 1;
                    }
                }
                else if (n == 0) {
                        /*4connection closed by client */
                    close(sockfd);
                    login[sockfd].username = "";
                    client[i].fd = -1;
                    cout << "file descriptor " << sockfd << " has been closed." << endl;
                }
                else
                    if (parse(sockfd, buf, n, users, login) < 0) {
                        close(sockfd);
                        client[i].fd = -1;
                        cout << "file descriptor " << sockfd << " has been closed by exit." << endl;
                    };

                if (--nready <= 0)
                    break;				/* no more readable descriptors */
            }
        }
    }
    cout << "Server closed." << endl;
    return 0;
}

int parse(
    int sockfd,
    char *readbuf,
    ssize_t n,
    vector<User> &users,
    vector<FDdata> &login
) {
    //cout << "read: " << n << endl;
    readbuf[n] = '\0';
    stringstream ss;
    ss << readbuf;
    string temp;
    string msg;
    vector<string> split;
    while(1) {
        ss >> temp;
        if (ss.fail()) break;
        split.push_back(temp);
        temp.clear();
    }

    if (split.empty()) return 0;

    int ret = 0;
    if (split[0] == "register") parseRegister(sockfd, users, login, split);
    else if (split[0] == "login") parseLogin(sockfd, users, login, split);
    else if (split[0] == "logout") parseLogout(sockfd, users, login, split);
    else if (split[0] == "exit") ret = parseExit(sockfd, login, split);
    else if (split[0] == "enter-chat-room") parseEnterChatRoom(sockfd, users, login, split);


    //write(sockfd, readbuf, n);
    return ret;
}

void parseRegister(
    int sockfd,
    vector<User> &users,
    vector<FDdata> &login,
    vector<string> &split
) {
    string msg;
    if (split.size() == 3) { // correct input format
        bool registered = false;
        for (auto u : users) {
            if (split[1] == u.username) { // username has been registered
                registered = true;
                break;
            }
        }
        if (!registered) {
            users.push_back(User(split[1], split[2]));
            msg += "Register successfully.\n% ";
            write(sockfd, msg.c_str(), msg.length());
        }
        else {
            msg += "Username is already used.\n% ";
            write(sockfd, msg.c_str(), msg.length());
        }
    }
    else { // wrong input format
        msg += "Usage: register <username> <password>\n% ";
        write(sockfd, msg.c_str(), msg.length());
    }
}

void parseLogin(
    int sockfd,
    vector<User> &users,
    vector<FDdata> &login,
    vector<string> &split
) {
    string msg;
    if (split.size() != 3) {
        msg += "Usage: login <username> <password>\n% ";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (login[sockfd].username != "") { // already login
        msg += "Please logout first.\n% ";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    for(int i = 2; i <= maxi; i++) {
        if (login[client[i].fd].username == split[1]) {
            msg += "Please logout first.\n% ";
            write(sockfd, msg.c_str(), msg.length());
            return;
        }
    }
    bool userExist = false;
    for (auto u : users) {
        if (split[1] == u.username) {
            userExist = true;
            break;
        }
    }
    if (!userExist) {
        msg += "Login failed.\n% ";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (blacklist.find(split[1]) != blacklist.end()) {
        msg += "We don't welcome ";
        msg += split[1];
        msg += "!\n% ";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    for (auto u : users) {
        if (split[1] == u.username && split[2] == u.password) {
            cout << u.username << ": Login successful." << endl;
            login[sockfd].username = u.username;
            break;
        }
    }
    if (login[sockfd].username != "") {
        msg += "Welcome, ";
        msg += split[1];
        msg += ".\n% ";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    else {
        msg += "Login failed.\n% ";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
}

void parseLogout(
    int sockfd,
    vector<User> &users,
    vector<FDdata> &login,
    vector<string> &split
) {
    string msg;
    if (split.size() == 1) {
        if (login[sockfd].username != "") {
            msg += "Bye, ";
            msg += login[sockfd].username;
            msg += ".\n% ";
            write(sockfd, msg.c_str(), msg.length());
            login[sockfd].username = "";
            login[sockfd].inChatRoom = false;
        }
        else {
            msg += "Please login first.\n% ";
            write(sockfd, msg.c_str(), msg.length());
        }
    }
    else {
        msg += "Usage: logout\n% ";
        write(sockfd, msg.c_str(), msg.length());
    }
}

int parseExit(
    int sockfd,
    vector<FDdata> &login,
    vector<string> &split
) {
    string msg;
    if (split.size() == 1) {
        if (login[sockfd].username != "") {
            msg += "Bye, ";
            msg += login[sockfd].username;
            msg += ".\n";
            write(sockfd, msg.c_str(), msg.length());
            login[sockfd].username = "";
            login[sockfd].inChatRoom = false;
        }
    }
    else {
        msg += "Usage: exit\n% ";
        write(sockfd, msg.c_str(), msg.length());
    }
    return -1;
}

void parseEnterChatRoom(
    int sockfd,
    vector<User> &users,
    vector<FDdata> &login,
    vector<string> &split
) {
    string msg;
    if (split.size() == 3) {
        if (isNumber(split[1]) && stoi(split[1]) > 0 && stoi(split[1]) < 65536) {
            if (isNumber(split[2]) && stoi(split[2]) > 0 && stoi(split[2]) < 3) {
                if (login[sockfd].username != "") { //success
                    login[sockfd].udpaddr.sin_port = htons(stoi(split[1]));
                    login[sockfd].version = stoi(split[2]);
                    login[sockfd].inChatRoom = true;
                    //socklen_t clilen = sizeof(login[sockfd].udpaddr);
                    //sendto(sockfd, "hello", strlen("hello"), 0, (sockaddr *) &(login[sockfd].udpaddr), clilen);
                    msg += "Welcome to public chat room.\nPort:";
                    msg += split[1];
                    msg += "\nVersion:";
                    msg += split[2];
                    msg += "\n";
                    msg += chatHistory;
                }
                else msg += "Please login first.\n";
            }
            else {
                msg += "Version ";
                msg += split[2];
                msg +=" is not supported.\n";
            }
        }
        else {
            msg += "Port ";
            msg += split[1];
            msg += " is not valid.\n";
        }
    }
    else msg += "Usage: enter-chat-room <port> <version>\n";

    msg += "% ";
    write(sockfd, msg.c_str(), msg.length());
}

void filter(char *namec, char *msg) {
    char *pch;
    string name(namec);
    char orimsg[MAXLINE];
    memset(orimsg, 0, sizeof(orimsg));
    strcpy(orimsg, msg);
    for(auto f : filterWords) {
        char stars[strlen(f)];
        memset(stars, '*', strlen(f));
        pch = strstr(msg, f);
        while(pch != NULL) {
            strncpy(pch, stars, strlen(f));
            pch = strstr(msg, f);
        }
    }
    if (strcmp(msg, orimsg) != 0) {
        for(auto u = users.begin(); u != users.end(); u++) {
            if (name == u->username) {
                u->black += 1;
                break;
            }
        }
    }
}

bool isNumber(const string& str){
    return str.find_first_not_of("0123456789") == string::npos;
}

int char_to_int(char *c) {
    int t = 0;
    for (int i = 0; i < strlen(c); i++) {
        t *= 10;
        t += c[i] - '0';
    }
    return t;
}

string letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

string base64Encode(string in) {
    string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val<<8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(letters[(val>>valb)&0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        out.push_back(letters[((val<<8)>>(valb+8))&0x3F]);
    while(out.size()%4)
        out.push_back('=');
    return out;
}

string base64Decode(string in) {
    string out;
    vector<int> T(256,-1);
    for (int i = 0; i < 64; i++) T[letters[i]] = i;

    int val = 0, valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val<<6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val>>valb)&0xFF));
            valb -= 8;
        }
    }
    return out;
}

