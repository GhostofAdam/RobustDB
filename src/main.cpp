#include "backend/DBMS.hpp"

bool initMode = false;

#ifdef __cplusplus
extern "C" char start_parse(const char *expr_input);
#endif
bool noCheck = false;
int main(int argc, char const *argv[]) {
    if (argc == 2 && strcmp("noCheck", argv[1]) == 0){
        noCheck = true;
    }
    printf("Starting RobustDB...\n");
    start_parse(nullptr);
    return 0;
}