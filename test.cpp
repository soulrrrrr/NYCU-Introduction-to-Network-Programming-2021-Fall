#include <iostream>
using namespace std;

void call(char *msg) {
    cout << "func: " << sizeof(msg) << endl;
}

int main () {
    char msg[100];
    call(msg);
    cout << "main: " << sizeof(msg) << endl;
    return 0;
}

