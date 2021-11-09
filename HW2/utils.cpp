#include <string>
#include <cstring>
#include "utils.h"
using namespace std;

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