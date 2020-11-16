#include "Database.hpp"
#include "utils/type.hpp"

class DBMS{
public:
    DBMS();
    static DBMS *getInstance();
    void exit();
    bool requireDbOpen();
    void switchToDB(const string name);
    void createTable(const table_def table);
    void dropDB(const string db_name);
    void dropTable(const string table);
    void listTables();
private:
    Database *current;
};