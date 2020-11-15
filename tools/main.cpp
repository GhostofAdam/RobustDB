#include "../include/DBMS.hpp"

bool initMode = false;

int main(int argc, char const *argv[]) {
    if (argc == 3 && strcmp("init", argv[2]) == 0)  
        initMode = true;
    DBMS::getInstance()->switchToDB(argv[1]);
    return 0;
}