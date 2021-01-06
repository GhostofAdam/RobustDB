#include "common.hpp"
#include "Index.hpp"
#include "bufmanager/BufPageManager.hpp"

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
    int8_t columnTot, primaryCount, checkTot, foreignKeyTot;
    int pageTot, recordByte, dataArrUsed;
    RID_t nextAvail, notNull, hasIndex, isPrimary;

    char columnName[MAX_COLUMN_SIZE][MAX_NAME_LEN];
    int columnOffset[MAX_COLUMN_SIZE];
    ColumnType columnType[MAX_COLUMN_SIZE];
    int columnLen[MAX_COLUMN_SIZE];
    int defaultOffset[MAX_COLUMN_SIZE];
    Check checkList[MAX_CHECK];
    ForeignKey foreignKeyList[MAX_FOREIGN_KEY];
    char dataArr[MAX_DATA_SIZE];
};
class Table{
    friend class Database;
public:
    void allocPage();
    char *getRecordTempPtr(RID_t rid);
    int getColumnOffset(int col){return head.columnOffset[col];}
    ColumnType getColumnType(int col){return head.columnType[col];}
    Table();
    ~Table();
    char *select(RID_t rid, int col);
    int getFooter(char *page, int idx);
    bool addColumn(const char *name, ColumnType type, int size,
                  bool notNull, bool hasDefault, const char *data);
    // return -1 if not found
    int getColumnID(const char *name);
    void setPrimary(int col);
    void addForeignKeyConstraint(unsigned int col, unsigned int foreignTableId, unsigned int foreignColId);
    void createIndex(int col);
    void dropIndex(int col);
    bool hasIndex(int col){return (head.hasIndex & (1 << col)) != 0;}
    int getColumnCount(){return head.columnTot;}
    void dropRecord(RID_t rid);
private:
    TableHead head;
    string tableName;
    vector<Index> colIndex;
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