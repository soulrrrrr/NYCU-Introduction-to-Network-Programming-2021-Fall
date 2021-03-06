#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <string>       // std::string
#include <sstream>      // std::stringstream
#include <poll.h>
#include <ctime>        // date
#include "structs.h"
#include "parse.h"
#include "utils.h"

using namespace std;

int parse(
    int sockfd,
    char *readbuf,
    ssize_t n,
    vector<User> &users,
    vector<string> &login,
    vector<Board> &boards,
    vector<Post> &posts
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
    else if (split[0] == "logout") parseLogout(sockfd, users, login);
    else if (split[0] == "exit") ret = parseExit(sockfd, login);
    else if (split[0] == "create-board") parseCreateBoard(sockfd, users, login, split, boards);
    else if (split[0] == "create-post") parseCreatePost(sockfd, users, login, split, boards, posts);
    else if (split[0] == "list-board") parseListBoard(sockfd, boards);
    else if (split[0] == "list-post") parseListPost(sockfd, split, boards, posts);
    else if (split[0] == "read") parseRead(sockfd, split, boards, posts);
    else if (split[0] == "delete-post") parseDeletePost(sockfd, login, split, posts);
    else if (split[0] == "update-post") parseUpdatePost(sockfd, login, split, posts);
    else if (split[0] == "comment") parseComment(sockfd, login, split, posts);

    //write(sockfd, readbuf, n);
    return ret;
}

void parseRegister(
    int sockfd,
    vector<User> &users,
    vector<string> &login,
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
            msg += "Register successfully.\n";
            write(sockfd, msg.c_str(), msg.length());
        }
        else {
            msg += "Username is already used.\n";
            write(sockfd, msg.c_str(), msg.length());
        }
    }
    else { // wrong input format
        msg += "Usage: login <username> <password>\n";
        write(sockfd, msg.c_str(), msg.length());
    }
}

void parseLogin(
    int sockfd,
    vector<User> &users,
    vector<string> &login,
    vector<string> &split
) {
    string msg;
    if (split.size() == 3) {
        if (login[sockfd] != "") { // already login
            msg += "Please logout first.\n";
            write(sockfd, msg.c_str(), msg.length());
            return;
        }
        for (auto u : users) {
            if (split[1] == u.username && split[2] == u.password) { // same username and password : login succeed
                for (auto nowLogin : login) {
                    if (nowLogin == u.username) {
                        msg += "Please logout first.\n";
                        write(sockfd, msg.c_str(), msg.length());
                        return;
                    }
                }
                cout << u.username << ": Login successful." << endl;
                login[sockfd] = u.username;
                break;
            }
        }
        if (login[sockfd] != "") {
            msg += "Welcome, ";
            msg += split[1];
            msg += ".\n";
            write(sockfd, msg.c_str(), msg.length());
        }
        else {
            msg += "Login failed.\n";
            write(sockfd, msg.c_str(), msg.length());
        }
    }
    else {
        msg += "Usage: login <username> <password>\n";
        write(sockfd, msg.c_str(), msg.length());
    }
}

void parseLogout(
    int sockfd,
    vector<User> &users,
    vector<string> &login
) {
    string msg;
    if (login[sockfd] != "") {
        msg += "Bye, ";
        msg += login[sockfd];
        msg += ".\n";
        write(sockfd, msg.c_str(), msg.length());
        login[sockfd] = "";
    }
    else {
        msg += "Please login first.\n";
        write(sockfd, msg.c_str(), msg.length());
    }
}

int parseExit(
    int sockfd,
    vector<string> &login
) {
    if (login[sockfd] != "") {
        login[sockfd] = "";
    }
    return -1;
}

void parseCreateBoard(
    int sockfd,
    vector<User> &users,
    vector<string> &login,
    vector<string> &split,
    vector<Board> &boards
) {
    string msg;
    if (split.size() != 2) { // fail(0)
        msg += "Usage: create-board <name>\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (login[sockfd] == "") { // fail(1)
        msg += "Please login first.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    for (auto b = boards.begin(); b != boards.end(); b++) {
        if (b->name == split[1]) { // fail(2)
            msg += "Board already exists.\n";
            write(sockfd, msg.c_str(), msg.length());
            return;
        }
    }
    boards.push_back(Board(boards.size()+1, split[1], login[sockfd])); // success
    msg += "Create board successfully.\n";
    write(sockfd, msg.c_str(), msg.length());
    return;
}

void parseCreatePost(
    int sockfd,
    vector<User> &users,
    vector<string> &login,
    vector<string> &split,
    vector<Board> &boards,
    vector<Post> &posts
) {
    string msg;
    string title, cont;
    int titleIndex=0, contentIndex=0;
    //cout << "split: " << split.size() << endl;
    for (int i = 0; i < split.size(); i++) {
        if (split[i] == "--title") {
            titleIndex = i;
        }
        else if (split[i] == "--content") {
            contentIndex = i;
        }
    }

    bool title_content_swapped = false;
    if (titleIndex > contentIndex) {
        swap(titleIndex, contentIndex);
        title_content_swapped = true;
    }

    if (split.size() < 6 || !titleIndex || !contentIndex) { // fail(0)
        msg += "Usage: create-post <board-name> --title <title> --content <content>\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (login[sockfd] == "") { // fail(1)
        msg += "Please login first.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    bool boardExists = false;
    for (auto b = boards.begin(); b != boards.end(); b++) {
        if (b->name == split[1]) { 
            boardExists = true;
            break;
        }
    }
    if (!boardExists) { // fail(2)
        msg += "Board does not exist.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    
    title = split[titleIndex+1];
    for(int i = titleIndex+2; i < contentIndex; i++) {
        title += " ";
        title += split[i];
    }
    cont = split[contentIndex+1];
    for(int i = contentIndex+2; i < split.size(); i++) {
        cont += " ";
        cont += split[i];
    }

    if (title_content_swapped)
        swap(title, cont);

    int t;
    while ((t = cont.find("<br>")) != -1) {
        cont.erase(t+1, 3);
        cont.replace(t, 1, "\n");
    }

    time_t now = time(0);
    tm *ltm = localtime(&now);
    string date = to_string(1 + ltm->tm_mon);
    date += "/";
    date += to_string(ltm->tm_mday);
    int sn = posts.size()+1;
    //cout << "sn: " << sn << endl;

    for (auto b = boards.begin(); b != boards.end(); b++) {
        if (b->name == split[1]) { 
            posts.push_back(Post(sn, title, login[sockfd], date, cont));
            //cout << "POSTS:" << posts.size() << endl;
            (b->postNum).push_back(sn-1);
            // for (int i = 0; i < (b->postNum).size(); i++) {
            //     cout << "in a: " << posts[(b->postNum)[i]].SN << endl;
            // }
            
            break;
        }
    }
    msg += "Create post successfully.\n";
    write(sockfd, msg.c_str(), msg.length());
    return;
}

void parseListBoard(
    int sockfd,
    vector<Board> &boards
) {
    string msg;
    msg += "Index Name Moderator\n";
    for (auto b : boards) {
        msg += to_string(b.index);
        msg += " ";
        msg += b.name;
        msg += " ";
        msg += b.moderator;
        msg += "\n";
    }
    write(sockfd, msg.c_str(), msg.length());
    return;
}

void parseListPost(
    int sockfd,
    vector<string> &split,
    vector<Board> &boards,
    vector<Post> &posts
) {
    string msg;
    if (split.size() != 2) { // fail(0)
        msg += "Usage: list-post <board-name>\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    bool boardExists = false;
    for (auto b = boards.begin(); b != boards.end(); b++) {
        if (b->name == split[1]) { 
            boardExists = true;
            break;
        }
    }
    if (!boardExists) { // fail(2)
        msg += "Board does not exist.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    msg += "S/N Title Author Date\n";
    for (auto b = boards.begin(); b != boards.end(); b++) {
        if (b->name == split[1]) {
            for (auto num : b->postNum) {
                Post* p = &posts[num];
                if (!p->deleted) {
                    msg += to_string(p->SN);
                    msg += " ";
                    msg += p->title;
                    msg += " ";
                    msg += p->author;
                    msg += " ";
                    msg += p->date;
                    msg += "\n";
                }
            }
            break;
        }
    }
    
    write(sockfd, msg.c_str(), msg.length());
    return;
}

void parseRead(
    int sockfd,
    vector<string> &split,
    vector<Board> &boards,
    vector<Post> &posts
) {
    string msg;
    if (split.size() != 2 ||
        !isNumber(split[1])) { // fail(0)
        msg += "Usage: read <post-S/N>\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (stoi(split[1]) > posts.size() || posts[stoi(split[1])-1].deleted) { //fail
        msg += "Post does not exist.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    Post *p = &(posts[stoi(split[1])-1]);
    msg += "Author: ";
    msg += p->author;
    msg += "\nTitle: ";
    msg += p->title;
    msg += "\nDate: ";
    msg += p->date;
    msg += "\n--\n";
    msg += p->content;
    msg += "\n--\n";
    for (auto c : p->comments) {
        msg += c.user;
        msg += ": ";
        msg += c.comment;
        msg += "\n";
    }
    write(sockfd, msg.c_str(), msg.length());
    return;
}

void parseDeletePost(
    int sockfd,
    vector<string> &login,
    vector<string> &split,
    vector<Post> &posts
) {
    string msg;
    if (split.size() != 2 ||
        !isNumber(split[1])) { // fail(0)
        msg += "Usage: delete-post <post-S/N>\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (login[sockfd] == "") { // fail(1)
        msg += "Please login first.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (stoi(split[1]) > posts.size() || posts[stoi(split[1])-1].deleted) { // fail(2)
        msg += "Post does not exist.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (login[sockfd] != posts[stoi(split[1])-1].author) { // fail(3)
        msg += "Not the post owner.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    posts[stoi(split[1])-1].deleted = true;
    msg += "Delete successfully.\n";
    write(sockfd, msg.c_str(), msg.length());
    return;
}

void parseUpdatePost(
    int sockfd,
    vector<string> &login,
    vector<string> &split,
    vector<Post> &posts
) {
    string msg;
    if (split.size() < 4 ||
        !isNumber(split[1]) ||
        split[2] != "--title" &&
        split[2] != "--content") { // fail(0)
        msg += "Usage: update-post <post-S/N> --title/content <new>\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (login[sockfd] == "") { // fail(1)
        msg += "Please login first.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (stoi(split[1]) > posts.size() ||
        posts[stoi(split[1])-1].deleted) { // fail(2)
        msg += "Post does not exist.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (login[sockfd] != posts[stoi(split[1])-1].author) { // fail(3)
        msg += "Not the post owner.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }

    string change = split[3];
    for(int i = 4; i < split.size(); i++) {
        change += " ";
        change += split[i];
    }
    if (split[2] == "--title") {
        posts[stoi(split[1])-1].title = change;
    }
    else {
        int t;
        while ((t = change.find("<br>")) != -1) {
            change.erase(t+1, 3);
            change.replace(t, 1, "\n");
        }
        posts[stoi(split[1])-1].content = change;
    }
    msg += "Update successfully.\n";
    write(sockfd, msg.c_str(), msg.length());
    return;
}

void parseComment(
    int sockfd,
    vector<string> &login,
    vector<string> &split,
    vector<Post> &posts
) {
    string msg;
    if (split.size() < 3 ||
        !isNumber(split[1])) { // fail(0)
        msg += "Usage: comment <post-S/N> <comment>\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (login[sockfd] == "") { // fail(1)
        msg += "Please login first.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    if (stoi(split[1]) > posts.size() ||
        posts[stoi(split[1])-1].deleted) { // fail(2)
        msg += "Post does not exist.\n";
        write(sockfd, msg.c_str(), msg.length());
        return;
    }
    string change = split[2];
    for(int i = 3; i < split.size(); i++) {
        change += " ";
        change += split[i];
    }
    posts[stoi(split[1])-1].comments.push_back(Comment(login[sockfd], change));
    msg += "Comment successfully.\n";
    write(sockfd, msg.c_str(), msg.length());
    return;
}
