#include "common.hpp"
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
    void loadIndex();
    void storeIndex();
    void dropIndex();
    
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
    int addPrimary(const char *col, const char* pk_name = nullptr);
    int dropPrimary_byname(const char *col);
    int dropForeignByName(const char *col);
    void dropPrimary();
    void setPrimary(int col);
    void addForeignKeyConstraint(unsigned int col, unsigned int foreignTableId, unsigned int foreignColId, const char* fk_name = nullptr);
    void dropForeign();
    int getColumnCount(){return head.columnTot;}
    int getFastCmp(RID_t rid, int col);
    bool getIsNull(RID_t rid, int col);
    void eraseColIndex(RID_t rid, int col);
    void insertColIndex(RID_t rid, int col);
    void dropRecord(RID_t rid);
    void printTableDef();
    bool insert2Buffer(int col, const char *data);
    bool insert2Record();
    void clearBuffer();
    void resetBuffer();
    void createIndex(int col, char *name = nullptr);
    void dropIndex(char *name);
    bool hasIndex(int col){return (head.hasIndex & (1 << col)) != 0;}
    void initTempRecord();
    void clearTempRecord();
    void setTempRecordNull(int col);
    void changeColumn(const char *col, struct column_defs *col_def);
    std::string setTempRecord(int col, const char *data);
    std::string insertTempRecord();
    std::string checkRecord();
    std::string loadRecordToTemp(RID_t rid, char *page, int offset);
    std::string modifyRecordNull(RID_t rid, int col);
    std::string modifyRecord(RID_t rid, int col, char *data);
    bool isPrimary(int col);
    bool checkPrimary();
    std::string checkForeignKeyConstraint();
    RID_t selectIndexLowerBoundEqual(int col, const char *data);
    RID_t selectIndexLowerBound(int col, const char *data);
    RID_t selectIndexLowerBoundNull(int col);
    RID_t selectIndexNext(int col);
    RID_t selectIndexNextEqual(int col);
    RID_t selectIndexUpperBound(int col, const char *data);
    RID_t selectIndexUpperBoundNull(int col);
    RID_t selectReveredIndexNext(int col);
    string tableName;

};