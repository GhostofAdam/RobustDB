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
    Table *getTableByName(const std::string &name);

private:
    bool ready;
    string dbName;
    vector<string> tableName;
    vector<Table*> table;
    size_t tableSize;
};