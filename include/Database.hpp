#include "Table.hpp"
class Database{
public:
    Database();
    ~Database();
    bool isOpen();
    void open(const string &name);
    void close();
    void drop();
    void create(const string &name);
    Table *createTable(const string &name);
    void dropTableByName(const string &name);
    vector<string> getTableNames();
    string getDBName();
    Table *getTableByName(const char* &name);
    bool setPrimaryKey(Table* tab, const char* columnName);
    bool setForeignKey(Table* tab, const char* columnName, const char* foreign_table_name, const char* foreign_column_name);
private:
    bool ready;
    string dbName;
    vector<string> tableName;
    vector<Table*> table;
    size_t tableSize;
};