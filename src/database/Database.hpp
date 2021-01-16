#ifndef __DATABASE_HPP__
#define __DATABASE_HPP__
#include "../table/Table.hpp"
#include "../table/ForeignerKey.hpp"
#include "../table/PrimaryKey.hpp"
#include "../table/TableIndex.hpp"
#include "../table/Record.hpp"

enum {
    EXCEPTION_NONE,
    EXCEPTION_DIFF_TYPE,
    EXCEPTION_ILLEGAL_OP,
    EXCEPTION_UNIMPLEMENTED,
    EXCEPTION_COL_NOT_UNIQUE,
    EXCEPTION_UNKNOWN_COLUMN,
    EXCEPTION_DATE_INVALID,
    EXCEPTION_WRONG_DATA_TYPE
};

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
    int getTableId(const string &name);
    Table *getTableById(const size_t id);
    Table *getTableByName(const string &name);
    void renameTable(const char *old_table, const char *new_table);
    bool setPrimaryKey(Table* tab, const char* columnName);
    bool setForeignKey(Table* tab, const char* columnName, const char* foreign_table_name, const char* foreign_column_name);
private:
    bool ready;
    string dbName;
    vector<string> tableName;
    vector<Table*> table;
};
#endif