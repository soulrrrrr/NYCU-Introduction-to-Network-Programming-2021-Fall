#ifndef STRUCTS_H
#define STRUCTS_H

#include <vector>
#include <string>
using namespace std;

struct Comment{
    string user;
    string comment;
    Comment(){}
    Comment(string user, string comment) : user(user), comment(comment) {}
};

struct Post{
    int SN;
    string title;
    string author;
    string date;
    string content;
    bool deleted;
    vector<Comment> comments;
    Post() {}
    Post(int SN, string title, string author, string date, string content) : SN(SN), title(title), author(author), date(date), content(content) {
        deleted = false;
    }
};

struct Board{
    int index;
    string name;
    string moderator;
    vector<int> postNum;
    Board() {}
    Board(int index, string name, string moderator) : index(index), name(name), moderator(moderator) {}
};

struct User{
    string username;
    string password;
    User() {}
    User(string username, string password): username(username), password(password) {}
};

#endif