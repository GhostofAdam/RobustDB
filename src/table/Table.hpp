#ifndef __TABLE_HPP__
#define __TABLE_HPP__
#include "../utils/common.hpp"
#include "../index/Index.hpp"
#include "Compare.hpp"
#include "../bufmanager/BufPageManager.hpp"
#include "../parser/type_def.hpp"

extern bool noCheck;

struct ForeignKey {
    unsigned int col;
    unsigned int foreign_table_id;
    unsigned int foreign_col;
    char name[MAX_NAME_LEN];
};

struct TableHead {
    int8_t columnTot;
    int8_t primaryCount;
    int8_t foreignKeyTot;
    int pageTot;
    int recordByte;
    int dataArrUsed;
    unsigned int nextAvail;
    unsigned int notNull;
    unsigned int hasIndex;
    unsigned int isPrimary;
    char columnName[MAX_COLUMN_SIZE][MAX_NAME_LEN];
    int columnOffset[MAX_COLUMN_SIZE];
    ColumnType columnType[MAX_COLUMN_SIZE];
    int columnLen[MAX_COLUMN_SIZE];
    int defaultOffset[MAX_COLUMN_SIZE];
    ForeignKey foreignKeyList[MAX_FOREIGN_KEY];
    char dataArr[MAX_DATA_SIZE];
    char pkName[MAX_COLUMN_SIZE][MAX_NAME_LEN];
    char indexName[MAX_COLUMN_SIZE][MAX_NAME_LEN];
};
class Table{
    friend class Database;
    friend class ForeignerKey;
    friend class PrimaryKey;
    friend class TableIndex;
    friend class Record;
private:
    TableHead head;
    
    Index colIndex[MAX_COLUMN_SIZE];
    int fileID, permID;
    char *buf;
    bool ready;
    void create(const char* tableName);
    void open(const char* tableName);
    void close();
    void drop();
    
public:
    Table();
    ~Table();
    void allocPage();
    std::string getTableName();
    RID_t getNext(RID_t rid);
    char *getRecordTempPtr(RID_t rid);
    int getColumnOffset(int col){return head.columnOffset[col];}
    ColumnType getColumnType(int col){return head.columnType[col];}
    char *select(RID_t rid, int col);
    int getFooter(char *page, int idx);
    void inverseFooter(const char *page, int idx);
    int addColumn(const char *name, ColumnType type, bool notNull, bool hasDefault, expr_node *data);
    int dropColumn(const char *name);
    int renameColumn(const char *old_col, const char *new_col);
    int getColumnID(const char *name);
    char *getColumnName(int col);
    int getColumnCount(){return head.columnTot;}
    int getFastCmp(RID_t rid, int col);
    bool getIsNull(RID_t rid, int col);
    bool hasIndex(int col){return (head.hasIndex & (1 << col)) != 0;}
    void dropRecord(RID_t rid);
    void printTableDef();
    bool insert2Buffer(int col, const char *data = nullptr);
    bool insert2Record();
    void clearBuffer();
    void resetBuffer();
    
    void changeColumn(const char *col, struct column_defs *col_def);

    std::string checkRecord();
    std::string checkForeignKeyConstraint();
    string tableName;

};
#endif