#include "common.hpp"
#include "Index.hpp"
#include "Compare.hpp"
#include "../bufmanager/BufPageManager.hpp"
#include "../parser/type_def.hpp"

struct Check {
    int col;
    int offset;
    OpType op;
    RelType rel;
};

struct ForeignKey {
    unsigned int col;
    unsigned int foreign_table_id;
    unsigned int foreign_col;
};

struct TableHead {
    int8_t columnTot;
    int8_t primaryCount;
    int8_t checkTot;
    int8_t foreignKeyTot;
    int pageTot;
    int recordByte;
    int dataArrUsed;    // addColumn中使用,记录dataArr的offset
    unsigned int nextAvail;
    unsigned int notNull;
    unsigned int hasIndex;
    unsigned int isPrimary;
    char columnName[MAX_COLUMN_SIZE][MAX_NAME_LEN];
    int columnOffset[MAX_COLUMN_SIZE];
    ColumnType columnType[MAX_COLUMN_SIZE];
    int columnLen[MAX_COLUMN_SIZE];
    int defaultOffset[MAX_COLUMN_SIZE];
    Check checkList[MAX_CHECK];
    ForeignKey foreignKeyList[MAX_FOREIGN_KEY];
    char dataArr[MAX_DATA_SIZE];    // 存储default data值
};
class Table{
    friend class Database;
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
    int addPrimary(const char *col);
    int dropPrimary_byname(const char *col);
    void dropPrimary();
    void setPrimary(int col);
    void addForeignKeyConstraint(unsigned int col, unsigned int foreignTableId, unsigned int foreignColId);
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
    void createIndex(int col);
    void dropIndex(int col);
    bool hasIndex(int col){return (head.hasIndex & (1 << col)) != 0;}
    void initTempRecord();
    void clearTempRecord();
    void setTempRecordNull(int col);
    std::string setTempRecord(int col, const char *data);
    std::string insertTempRecord();
    std::string checkRecord();
    std::string loadRecordToTemp(RID_t rid, char *page, int offset);
    std::string modifyRecordNull(RID_t rid, int col);
    std::string modifyRecord(RID_t rid, int col, char *data);
    bool isPrimary(int col);
    bool checkPrimary();
    std::string checkValueConstraint();
    std::string checkForeignKeyConstraint();
    std::string genCheckError(int checkId);
    RID_t selectIndexLowerBoundEqual(int col, const char *data);
    RID_t selectIndexLowerBound(int col, const char *data);
    RID_t selectIndexLowerBoundNull(int col);
    RID_t selectIndexNext(int col);
    RID_t selectIndexNextEqual(int col);
    RID_t selectIndexUpperBound(int col, const char *data);
    RID_t selectIndexUpperBoundNull(int col);
    RID_t selectReveredIndexNext(int col);
    string tableName;
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
};