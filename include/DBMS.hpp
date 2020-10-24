#include "Database.hpp"
#include "common.hpp"
class DBMS{
public:
    DBMS();
    void exit();
    void switchToDB(const string name);
    void createTable(const string table);
    void dropDB(const string db_name);
    void dropTable(const string table);
    void listTables();
private:
    Database *current_;
};