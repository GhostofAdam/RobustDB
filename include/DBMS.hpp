#include "Database.hpp"
#include "common.hpp"

class DBMS{
public:
    DBMS();
    void exit();
    bool requredDbOpen();
    void switchToDB(const string name);
    void createTable(const table_def table);
    void dropDB(const string db_name);
    void dropTable(const string table);
    void listTables();
private:
    Database *current;
};