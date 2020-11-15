#include "common.hpp"
#include "setting.hpp"
#include "bufmanager/BufPageManager.h"

struct TableHead {
    int8_t columnTot, primaryCount, checkTot, foreignKeyTot;
    int pageTot, recordByte, dataArrUsed;
    unsigned int nextAvail, notNull, hasIndex, isPrimary;
};

class Table{
    friend class Database;
public:
    void allocPage();
    RID_t getNext(RID_t rid);
    void getRecord(RID_t rid, char *buf);
    void dropRecord(RID_t rid);
    int getRecordBytes();
private:
    TableHead head;
    string tableName;
    bool ready;
    void create(string tableName);
    void open(string tableName);
    void close();
    void drop();
};
