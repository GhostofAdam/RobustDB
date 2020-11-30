#include "Database.hpp"
#include "utils/type.hpp"

class DBMS{
public:
    DBMS();
    static DBMS getInstance(){
        static DBMS instance;
        return instance;
    }
    enum IDX_TYPE {
        IDX_NONE, IDX_LOWWER, IDX_UPPER, IDX_EQUAL
    };
    void exit();
    bool requireDbOpen();
    void switchToDB(const string name);
    void createTable(const table_def table);
    void dropDB(const string db_name);
    void dropTable(const string table);
    void listTables();
    void createIndex(column_ref *tb_col);
    void dropIndex(column_ref *tb_col);
private:
    Database *current;
};