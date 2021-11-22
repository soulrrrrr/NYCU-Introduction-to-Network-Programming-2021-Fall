#include <iostream>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h> // bzero
#include <unistd.h> // write, read
using namespace std;

int main(int argc, char** argv) {
    int sockfd;
    sockaddr_in servaddr;
    if (argc != 3) {
        cout << "usage: tcpcli <IPaddress> <port>\n";
        exit(0);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(stoi(argv[2]));
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    connect(sockfd, (sockaddr *) &servaddr, sizeof(servaddr));

    int maxfdp1, stdineof;
    fd_set rset;
    char sendline[4096], recvline[4096];
    FD_ZERO(&rset);
    while(1) {
        if (stdineof == 0)
            FD_SET(fileno(stdin), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(stdin), sockfd) + 1;
        //cout << "maxfdp1: " << maxfdp1;
        select(maxfdp1, &rset, NULL, NULL, NULL);

        int n;
        if (FD_ISSET(sockfd, &rset)) {
            if ((n = read(sockfd, recvline, 4096)) == 0) {
                if (stdineof == 1)
                    break;
                else {
                    cout << "Read error." << endl;
                    exit(0);
                }
            }
            write(fileno(stdout), recvline, n);
        }

        if (FD_ISSET(fileno(stdin), &rset)) {
            if ((n = read(fileno(stdin), sendline, 4096)) == 0) {
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(stdin), &rset);
                continue;
            }
            write(sockfd, sendline, n);
        }
    }
    exit(0);
}