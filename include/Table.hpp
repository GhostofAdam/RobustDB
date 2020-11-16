#include "common.hpp"
#include "Index.hpp"
#include "bufmanager/BufPageManager.hpp"
enum ColumnType {
    CT_INT, CT_VARCHAR, CT_FLOAT, CT_DATE
};

struct TableHead {
    int8_t columnTot, primaryCount, checkTot, foreignKeyTot;
    int pageTot, recordByte, dataArrUsed;
    unsigned int nextAvail, notNull, hasIndex, isPrimary;
};
class Table{
    friend class Database;
public:
    void allocPage();
    // RID_t getNext(RID_t rid);
    // void getRecord(RID_t rid, char *buf);
    // void dropRecord(RID_t rid);
    // int getRecordBytes();
    // void allocPage();
    // int addColumn(const char *name, ColumnType type, int size,
    //               bool notNull, bool hasDefault, const char *data);
    // void setPrimary(int columnID);
    // int getColumnCount();
    // // return -1 if not found
    // int getColumnID(const char *name);
    Table();
    ~Table();
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

bool operator<(const IndexKey &a, const IndexKey &b);
