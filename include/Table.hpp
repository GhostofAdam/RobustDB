#include "common.hpp"
#include "setting.hpp"

struct TableHead {
    int8_t columnTot, primaryCount, checkTot, foreignKeyTot;
    int pageTot, recordByte, dataArrUsed;
    unsigned int nextAvail, notNull, hasIndex, isPrimary;
};

class Table{
public:
    void allocPage();
    RID_t getNext(RID_t rid);
    void getRecord(RID_t rid, char *buf);
    void dropRecord(RID_t rid);
    int getRecordBytes();
private:
    TableHead head_;
    string name_;
    void create(string tableName);
    void open(string tableName);
    void close();
    void drop();
};
