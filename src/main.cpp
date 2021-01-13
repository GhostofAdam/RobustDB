#include "backend/DBMS.hpp"

bool initMode = false;

#ifdef __cplusplus
extern "C" char start_parse(const char *expr_input);
#endif

int main(int argc, char const *argv[]) {
    if (argc == 3 && strcmp("init", argv[2]) == 0)  
        initMode = true;
    printf("Starting RobustDB...\n");
    DBMS::getInstance()->switchToDB(argv[1]);
    start_parse(nullptr);
    return 0;
}