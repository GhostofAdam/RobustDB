#include "Table.hpp"
#include "RegisterManager.hpp"

bool operator<(const IndexKey &a, IndexKey &b) {
    assert(a.getPermID() == b.getPermID());
    assert(a.getCol() == b.getCol());
    
    if (a.getIsNull()) {
        if (b.getIsNull())  
            return a.getRid() < b.getRid();
        return false;
    }
    else if (b.getIsNull())
        return false;
    
    return a.getRid() < b.getRid();

}

Table::Table() {
    this->ready = false;
}

Table::~Table() {
    if (this->ready) 
        close();
}

void Table::create(const char* tableName) {
    assert(!this->ready);
    this->tableName = tableName;
    BufPageManager::getFileManager().createFile(tableName);
    this->fileID = BufPageManager::getFileManager().openFile(tableName);
    this->permID = BufPageManager::getFileManager().getFilePermID(fileID);
    BufPageManager::getInstance().allocPage(fileID, 0);
    RegisterManager::getInstance().checkIn(permID, this);
    this->ready = true;
    this->buf = nullptr;
    this->colIndex.clear();
    head.pageTot = 1;
    head.recordByte = 4;
    head.columnTot = 0;
    head.dataArrUsed = 0;
    head.nextAvail = (unsigned int) -1;
    head.notNull = 0;
    head.checkTot = 0;
    head.foreignKeyTot = 0;
    head.primaryCount = 0;
}

void Table::open(const char* tableName) {
    assert(!this->ready);
    this->tableName = tableName;
    this->fileID = BufPageManager::getFileManager().openFile(tableName);
    this->permID = BufPageManager::getFileManager().getFilePermID(fileID);
    RegisterManager::getInstance().checkIn(permID, this);
    int index = BufPageManager::getInstance().getPage(fileID, 0);
    memcpy(&head, BufPageManager::getInstance()._access(index), sizeof(TableHead));
    this->ready = true;
    this->buf = nullptr;
    for (auto &col: this->colIndex)
        col.clear();
    loadIndex();
}

void Table::close() {
    assert(this->ready);
    storeIndex();
    int index = BufPageManager::getInstance().getPage(fileID, 0);
    memcpy(BufPageManager::getInstance()._access(index), &head, sizeof(TableHead));
    BufPageManager::getInstance().markDirty(index);
    RegisterManager::getInstance().checkOut(permID);
    BufPageManager::getInstance().closeFile(fileID);
    BufPageManager::getFileManager().closeFile(fileID);
    this->ready = false;
}

void Table::drop() {
    assert(this->ready);
    dropIndex();
    RegisterManager::getInstance().checkOut(permID);
    BufPageManager::getInstance().closeFile(fileID, false);
    BufPageManager::getFileManager().closeFile(fileID);
    this->ready = false;
}

void Table::loadIndex() {
    for (int i = 0; i < head.columnTot; i++)
        if (head.hasIndex & (1 << i))
            colIndex[i].load(permID, i);
}

void Table::storeIndex() {
    for (int i = 0; i < head.columnTot; i++)
        if (head.hasIndex & (1 << i)) 
            colIndex[i].store(permID, i);
}

void Table::dropIndex() {
    for (int i = 0; i < head.columnTot; i++)
        if (head.hasIndex & (1 << i))
            colIndex[i].drop(permID, i);
}
char *Table::getRecordTempPtr(RID_t rid) {
    int pageID = rid / PAGE_SIZE;
    int offset = rid % PAGE_SIZE;
    assert(1 <= pageID && pageID < head.pageTot);
    auto index = BufPageManager::getInstance().getPage(fileID, pageID);
    char* page = (char *)BufPageManager::getInstance()._access(index);
    assert(getFooter(page, offset / head.recordByte));
    return page + offset;
}
//return 0 when null
//return value in tempbuf when rid = -1
char *Table::select(RID_t rid, int col) {
    char *ptr;
    if (rid != (RID_t) -1) {
        ptr = getRecordTempPtr(rid);
    } else {
        ptr = buf;
    }
    unsigned int &notNull = *(unsigned int *) ptr;
    char *buf;
    if ((~notNull) & (1 << col)) {
        return nullptr;
    }
    switch (head.columnType[col]) {
        case CT_INT:
        case CT_DATE:
        case CT_FLOAT:
            buf = new char[4];
            memcpy(buf, ptr + getColumnOffset(col), 4);
            return buf;
        case CT_VARCHAR:
            buf = new char[head.columnLen[col] + 1];
            strcpy(buf, ptr + getColumnOffset(col));
            return buf;
        default:
            assert(0);
    }
}
int Table::getFooter(char *page, int idx) {
    int u = idx / 32;
    int v = idx % 32;
    unsigned int tmp = *(unsigned int *) (page + PAGE_SIZE - PAGE_FOOTER_SIZE + u * 4);
    return (tmp >> v) & 1;
}
void Table::createIndex(int col) {
    //assert(head.pageTot == 1);
    assert((head.hasIndex & (1 << col)) == 0);
    head.hasIndex |= 1 << col;
}
void Table::dropIndex(int col) {
    assert((head.hasIndex & (1 << col)));
    head.hasIndex &= ~(1 << col);
    colIndex[col].drop(permID, col);
}
void Table::setPrimary(int col) {
    assert((head.notNull >> col) & 1);
    head.isPrimary |= (1 << col);
    ++head.primaryCount;
}
void Table::addForeignKeyConstraint(unsigned int col, unsigned int foreignTableId, unsigned int foreignColId) {
    assert(head.foreignKeyTot < MAX_FOREIGN_KEY);
    head.foreignKeyList[head.foreignKeyTot].col = col;
    head.foreignKeyList[head.foreignKeyTot].foreign_table_id = foreignTableId;
    head.foreignKeyList[head.foreignKeyTot].foreign_col = foreignColId;
    head.foreignKeyTot++;
}
// return -1 if not found
int Table::getColumnID(const char *name) {
    for (int i = 1; i < head.columnTot; i++)
        if (strcmp(head.columnName[i], name) == 0)
            return i;
    return -1;
}
bool operator<(const IndexKey &a, const IndexKey &b) {
    assert(a.permID == b.permID);
    assert(a.col == b.col);
    Table *tab = RegisterManager::getInstance().getPtr(a.permID);
    ColumnType tp = tab->getColumnType(a.col);

    //compare null
    if (a.isNull) {
        if (b.isNull) return a.rid < b.rid;
        return false;
    } else {
        if (b.isNull) return false;
    }

    //fast compare
    int tmp;
    switch (tp) {
        case CT_INT:
        case CT_DATE:
            tmp = Compare::sgn(a.fastCmp - b.fastCmp);
            if (tmp == -1) return true;
            if (tmp == 1) return false;
            return a.rid < b.rid;
        case CT_VARCHAR:
        case CT_FLOAT:
            tmp = Compare::sgn(a.fastCmp - b.fastCmp);
            if (tmp == -1) return true;
            if (tmp == 1) return false;
            break;
        default:
            assert(0);
    }

    char *x = tab->select(a.rid, a.col);
    char *y = tab->select(b.rid, b.col);
    int res = 0;
    switch (tp) {
        case CT_VARCHAR:
            res = Compare::compareVarcharSgn(x, y);
            break;
        case CT_FLOAT:
            res = Compare::compareFloatSgn(*(float *) x, *(float *) y);
        default:
            assert(0);
    }
    delete[] x;
    delete[] y;
    if (res < 0) return true;
    if (res > 0) return false;
    return a.rid < b.rid;
}