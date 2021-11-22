#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <vector> // std::vector
#include <algorithm> // std::sort
#include <poll.h>
#include "structs.h"
#include "parse.h"
#include "utils.h"

#define LISTENQ 10
#define MAXLINE 4096
#define INFTIM -1
using namespace std;

vector<Post> posts;
vector<Board> boards;
vector<User> users;
vector<string> login(OPEN_MAX, "");

int main(int argc, char** argv) {
    int i, maxi, serverfd, clientfd, sockfd;
    int	nready;
    ssize_t	n; /* read buffer size */
    fd_set rset, allset;
    char buf[MAXLINE];
    string msg;
    socklen_t clilen;
    pollfd		client[OPEN_MAX];
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

    if (listen(serverfd, LISTENQ) == -1) {
        cerr << "listen() error.\n";
        return 1;
    }

    client[0].fd = serverfd;
	client[0].events = POLLRDNORM;
	for (i = 1; i < OPEN_MAX; i++)
		client[i].fd = -1;		/* -1 indicates available entry */
	maxi = 0;					/* max index into client[] array */
    

    while(1) {
        nready = poll(client, maxi+1, INFTIM);
        
        if (client[0].revents & POLLRDNORM) {	/* new client connection */
			clilen = sizeof(cliaddr);
			clientfd = accept(serverfd, (sockaddr *) &cliaddr, &clilen);
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

            for (i = 1; i < OPEN_MAX; i++)
                if (client[i].fd < 0) {
                    client[i].fd = clientfd;	/* save descriptor */
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

        for (i = 1; i <= maxi; i++) {	/* check all clients for data */
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
                    login[sockfd] = "";
                    client[i].fd = -1;
                    cout << "file descriptor " << sockfd << " has been closed." << endl;
                }
                else
                    //write(sockfd, buf, n);
                    if (parse(sockfd, buf, n, users, login, boards, posts) < 0) {
                        close(sockfd);
                        client[i].fd = -1;
                        cout << "file descriptor " << sockfd << " has been closed by exit." << endl;
                    };

                msg = "% ";
                write(sockfd, msg.c_str(), msg.length());
                if (--nready <= 0)
                    break;				/* no more readable descriptors */
            }
        }
    }
    cout << "Server closed." << endl;
    return 0;
}

