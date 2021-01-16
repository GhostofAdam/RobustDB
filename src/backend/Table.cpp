
#include "../dbms/DBMS.hpp"
class DBMS;


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
    BufPageManager::getFileManager().createFile(tableName); // 创建table对应文件
    this->fileID = BufPageManager::getFileManager().openFile(tableName);  
    this->permID = BufPageManager::getFileManager().getFilePermID(this->fileID);
    BufPageManager::getInstance().allocPage(this->fileID, 0); // 为文件首页获取一个缓存中的页面
    RegisterManager::getInstance().checkIn(permID, this);
    this->ready = true;
    this->buf = nullptr;
    head.pageTot = 1;       // 对应文件页数
    head.recordByte = 4;
    head.columnTot = 0;     // table列数
    head.dataArrUsed = 0;
    head.nextAvail = (unsigned int) -1;
    head.notNull = 0;       // 规定非空列数量
    head.foreignKeyTot = 0; // 外键数量
    head.primaryCount = 0;  // 主键数量
    addColumn("RID", (ColumnType)CT_INT, true, false, nullptr);
    setPrimary(0);
    for (auto &col: colIndex) {
        col.clear();
    }
}

void Table::open(const char* tableName) {
    assert(!this->ready);
    this->tableName = tableName;
    this->fileID = BufPageManager::getFileManager().openFile(tableName);
    this->permID = BufPageManager::getFileManager().getFilePermID(fileID);
    RegisterManager::getInstance().checkIn(permID, this);
    int index = BufPageManager::getInstance().getPage(fileID, 0);   // 为文件首页在缓存中找到对应缓存页面
    memcpy(&(this->head), BufPageManager::getInstance().access(index), sizeof(TableHead));
    this->ready = true;
    this->buf = nullptr;
    for (auto &col: this->colIndex)
        col.clear();
    loadIndex();    // 加载各列信息
}

void Table::close() {
    assert(this->ready);
    storeIndex();
    int index = BufPageManager::getInstance().getPage(fileID, 0);
    memcpy(BufPageManager::getInstance().access(index), &(this->head), sizeof(this->head));

    BufPageManager::getInstance().markDirty(index);
    RegisterManager::getInstance().checkOut(permID);
    BufPageManager::getInstance().closeFile(fileID);
    BufPageManager::getFileManager().closeFile(fileID);
    this->ready = false;
    if (buf) {
        delete[] buf;
        buf = 0;
    }
}

void Table::drop() {
    assert(this->ready);
    dropIndex();    // 删除所有列
    RegisterManager::getInstance().checkOut(permID);
    BufPageManager::getInstance().closeFile(fileID, false);
    BufPageManager::getFileManager().closeFile(fileID);
    this->ready = false;
}

void Table::loadIndex() {
    for (int i = 0; i < head.columnTot; i++)
        if (head.hasIndex & (1 << i))   // bitmap
            colIndex[i].load(permID, i);    // 从文件加载对应列
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

void Table::allocPage() {
    auto index = BufPageManager::getInstance().allocPage(fileID, head.pageTot);
    auto buf = BufPageManager::getInstance().access(index);
    auto n = (PAGE_SIZE - PAGE_FOOTER_SIZE) / head.recordByte;
    n = (n < MAX_REC_PER_PAGE) ? n : MAX_REC_PER_PAGE;
    for (int i = 0, p = 0; i < n; i++, p += head.recordByte) {
        unsigned int &ptr = *(unsigned int *) (buf + p);
        ptr = head.nextAvail;
        head.nextAvail = (unsigned int) head.pageTot * PAGE_SIZE + p;
    }
    memset(buf + PAGE_SIZE - PAGE_FOOTER_SIZE, 0, PAGE_FOOTER_SIZE);
    BufPageManager::getInstance().markDirty(index);
    head.pageTot++;
}

RID_t Table::getNext(RID_t rid) {
    int page_id, id, n;
    n = (PAGE_SIZE - PAGE_FOOTER_SIZE) / head.recordByte;
    n = (n < MAX_REC_PER_PAGE) ? n : MAX_REC_PER_PAGE;
    if (rid == (RID_t) -1) {
        page_id = 0;
        id = n - 1;
    } else {
        page_id = rid / PAGE_SIZE;
        id = (rid % PAGE_SIZE) / head.recordByte;
    }
    int index = BufPageManager::getInstance().getPage(fileID, page_id);
    char *page =    BufPageManager::getInstance().access(index);

    while (true) {
        id++;
        if (id == n) {
            page_id++;
            if (page_id >= head.pageTot) return (RID_t) -1;
            index = BufPageManager::getInstance().getPage(fileID, page_id);
            page = BufPageManager::getInstance().access(index);
            id = 0;
        }
        if (getFooter(page, id)) return (RID_t) page_id * PAGE_SIZE + id * head.recordByte;
    }
}

std::string Table::getTableName() {
    assert(ready);
    return tableName;
}

char *Table::getRecordTempPtr(RID_t rid) {
    int pageID = rid / PAGE_SIZE;
    int offset = rid % PAGE_SIZE;
    assert(1 <= pageID && pageID < head.pageTot);
    auto index = BufPageManager::getInstance().getPage(fileID, pageID);
    auto page = BufPageManager::getInstance().access(index);
    assert(getFooter(page, offset / head.recordByte));
    return page + offset;
}

//return 0 when null
//return value in tempbuf when rid = -1
char *Table::select(RID_t rid, int col) {
    char *ptr;
    if (rid != (RID_t) -1)  ptr = getRecordTempPtr(rid);
    else ptr = buf;
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

void Table::inverseFooter(const char *page, int idx) {
    int u = idx / 32;
    int v = idx % 32;
    unsigned int &tmp = *(unsigned int *) (page + PAGE_SIZE - PAGE_FOOTER_SIZE + u * 4);
    tmp ^= (1u << v);
}

int Table::getFooter(char *page, int idx) {
    int u = idx / 32;
    int v = idx % 32;
    unsigned int tmp = *(unsigned int *) (page + PAGE_SIZE - PAGE_FOOTER_SIZE + u * 4);
    return (tmp >> v) & 1;
}

void Table::createIndex(int col, char *name) {
    assert((head.hasIndex & (1 << col)) == 0);
    if(name)
        strcpy(head.indexName[col], name);
    head.hasIndex |= 1 << col;
}

void Table::dropIndex(char *name) {
    for(int col = 0; col<head.columnTot;col++){
        if(strcmp(head.indexName[col],name) == 0){
            assert((head.hasIndex & (1 << col)));
            head.hasIndex &= ~(1 << col);
            colIndex[col].drop(permID, col);
        }
    }
}

int Table::getFastCmp(RID_t rid, int col) {
    char *p = select(rid, col);
    if (p == nullptr) return 0;
    int res = 0;
    float tmp;
    switch (head.columnType[col]) {
        case CT_INT:
        case CT_DATE:
            res = *(int *) p;
            break;
        case CT_FLOAT:
            tmp = *(float *) p;
            if (tmp > 2e9)
                res = (int) 2e9;
            else if (tmp < -2e9)
                res = (int) -2e9;
            else
                res = (int) tmp;
            break;
        case CT_VARCHAR:
            res = 0;
            for (size_t i = 0; i < 4; i++) {
                res = res * 256;
                if (i < strlen(p)) res += i;
            }
            break;
        default:
            assert(false);
    }
    delete[] p;
    return res;
}

bool Table::getIsNull(RID_t rid, int col) {
    char *p = select(rid, col);
    delete[] p;
    return p == nullptr;
}

void Table::eraseColIndex(RID_t rid, int col) {
    if (hasIndex(col)) {
        colIndex[col].erase(IndexKey(permID, rid, col, getFastCmp(rid, col), getIsNull(rid, col)));
    }
}

void Table::insertColIndex(RID_t rid, int col) {
    if (hasIndex(col)) {
        colIndex[col].insert(IndexKey(permID, rid, col, getFastCmp(rid, col), getIsNull(rid, col)));
    }
}

void Table::dropRecord(RID_t rid) {
    int pageID = rid / PAGE_SIZE;
    int offset = rid % PAGE_SIZE;
    for (int i = 0; i < head.columnTot; i++) {
        if (head.hasIndex & (1 << i)) eraseColIndex(rid, i);
    }
    int index = BufPageManager::getInstance().getPage(fileID, pageID);
    char *page = (char*)BufPageManager::getInstance().access(index);
    char *record = page + offset;
    unsigned int &next = *(unsigned int *) record;
    next = head.nextAvail;
    head.nextAvail = rid;
    inverseFooter(page, offset / head.recordByte);
    BufPageManager::getInstance().markDirty(index);
}

int Table::addColumn(const char *name, ColumnType type, bool notNull, bool hasDefault, expr_node *data){
    for (int i = 0; i < head.columnTot; i++)    // 检查是否存在重复的列
        if (strcmp(head.columnName[i], name) == 0)
            return -1;
    assert(head.columnTot < MAX_COLUMN_SIZE);
    int id = head.columnTot++;
    strcpy(head.columnName[id], name);
    head.columnType[id] = type;
    head.columnOffset[id] = head.recordByte;

    if (notNull) head.notNull |= (1 << id);
    
    head.defaultOffset[id] = -1;
    switch (type) {
        case CT_INT:
            head.recordByte += 4;
            head.columnLen[id] = 4;
            if (hasDefault) {
                head.defaultOffset[id] = head.dataArrUsed;
                memcpy(head.dataArr + head.dataArrUsed, &(data->literal_i), 4);
                head.dataArrUsed += 4;
            }
            break;
        case CT_FLOAT:
            head.recordByte += 4;
            head.columnLen[id] = 4;
            if (hasDefault) {
                head.defaultOffset[id] = head.dataArrUsed;
                memcpy(head.dataArr + head.dataArrUsed, &(data->literal_f), 4);
                head.dataArrUsed += 4;
            }
            break;
        case CT_DATE:
            head.recordByte += 4;
            head.columnLen[id] = 4;
            if (hasDefault) {
                head.defaultOffset[id] = head.dataArrUsed;
                memcpy(head.dataArr + head.dataArrUsed, &(data->literal_i), 4);
                head.dataArrUsed += 4;
            }
            break;
        case CT_VARCHAR:
            head.recordByte += MAX_DATA_LEN + 1;
            head.recordByte += 4 - head.recordByte % 4;
            head.columnLen[id] = MAX_DATA_LEN;
            if (hasDefault) {
                head.defaultOffset[id] = head.dataArrUsed;
                strcpy(head.dataArr + head.dataArrUsed, data->literal_s);
                head.dataArrUsed += MAX_DATA_LEN + 1;
            }
            break;
        default:
            assert(0);
    }
    assert(head.dataArrUsed <= MAX_DATA_SIZE);
    assert(head.recordByte <= PAGE_SIZE);
    printf("--Added Column: %s\n", name);
    return id;
}

bool Table::insert2Buffer(int col, const char *data){
    if (data == nullptr) {
        setTempRecordNull(col);
        return true;
    }
    if (buf == nullptr) {
        buf = new char[head.recordByte];
        initTempRecord();
    }
    unsigned int &notNull = *(unsigned int *) buf;
    switch (head.columnType[col]) {
        case CT_INT:
        case CT_DATE:
        case CT_FLOAT:
            memcpy(buf + head.columnOffset[col], data, 4);
            break;
        case CT_VARCHAR:
            if ((unsigned int) head.columnLen[col] < strlen(data)) {
                printf("[ERROR]Varchar too long\n");
                return false;
            }
            strcpy(buf + head.columnOffset[col], data);
            break;
        default:
            assert(0);
    }
    notNull |= (1u << col);
    return true;
}

bool Table::insert2Record(){
    assert(buf != nullptr);
    if (head.nextAvail == (RID_t) -1) {
        allocPage();
    }
    int rid = head.nextAvail;
    setTempRecord(0, (char *) &head.nextAvail);
    auto error = checkRecord();
    if (!error.empty()) {
        printf("%s", error.c_str());
        printf("[ERROR]Insert Error at RID %d\n", rid);
        return false;
    }
    int pageID = head.nextAvail / PAGE_SIZE;
    int offset = head.nextAvail % PAGE_SIZE;
    int index = BufPageManager::getInstance().getPage(fileID, pageID);
    char *page = (char*) BufPageManager::getInstance().access(index);
    head.nextAvail = *(unsigned int *) (page + offset);
    memcpy(page + offset, buf, head.recordByte);
    BufPageManager::getInstance().markDirty(index);
    inverseFooter(page, offset / head.recordByte);
    for (int i = 0; i < head.columnTot; i++) insertColIndex(rid, i);
    return true;
}
    
int Table::dropColumn(const char *name) {
    int id = -1;
    for (int i = 0; i < head.columnTot; i++) {
        if (strcmp(head.columnName[i], name) == 0)
            id = i;
    }
    if (id == -1){
        printf("[ERROR]Drop Column Error Column does not exits\n");
        return -1;
    }

    unsigned int notNull_new = 0;
    for (int i = 0; i < head.columnTot - 1; i++) {
        if (i < id) {
            if ((head.notNull >> i) & 0x1)
                notNull_new |= (1 << i);
        }
        else {
            if ((head.notNull >> (i + 1)) & 0x1)
                notNull_new |= (1 << i);
        }
    }
    head.notNull = notNull_new;

    unsigned int hasIndex_new = 0;
    for (int i = 0; i < head.columnTot - 1; i++) {
        if (i < id) {
            if ((head.hasIndex >> i) & 0x1)
                hasIndex_new |= (1 << i);
        }
        else {
            if ((head.hasIndex >> (i + 1)) & 0x1)
                hasIndex_new |= (1 << i);
        }
    }
    head.hasIndex = hasIndex_new;

    if ((head.isPrimary >> id) & 0x1) {
        head.primaryCount--;
    }
    unsigned int isPrimary_new = 0;
    for (int i = 0; i < head.columnTot - 1; i++) {
        if (i < id) {
            if ((head.isPrimary >> i) & 0x1)
                isPrimary_new |= (1 << i);
        }
        else {
            if ((head.isPrimary >> (i + 1)) & 0x1)
                isPrimary_new |= (1 << i);
        }
    }
    head.isPrimary = isPrimary_new;

    int8_t fkt = head.foreignKeyTot;
    bool fk_flag[fkt];
    for (int8_t i = 0; i < fkt; i++) {
        if (head.foreignKeyList[i].col == id)
            fk_flag[i] = false;
        else
            fk_flag[i] = true;
    }
    head.foreignKeyTot = 0;
    for (int8_t i = 0; i < fkt; i++) {
        if (fk_flag[i]) {
            head.foreignKeyList[head.foreignKeyTot] = head.foreignKeyList[i];
            head.foreignKeyTot++;
        }
    }

    for (int i = id; i < head.columnTot - 1; i++) {
        strcpy(head.columnName[i], head.columnName[i + 1]);
        head.columnOffset[i] = head.columnOffset[i + 1];
        head.columnType[i] = head.columnType[i + 1];
        head.columnLen[i] = head.columnLen[i + 1];
        head.defaultOffset[i] = head.defaultOffset[i + 1];
        strcpy(head.pkName[i], head.pkName[i + 1]);
    }
    head.columnTot--;
    printf("--Dropped column %s\n", name);
    return id;
}

int Table::renameColumn(const char *old_col, const char *new_col) {
    for (int i = 0; i < head.columnTot; i++) {
        if (strcmp(head.columnName[i], old_col) == 0) {
            printf("--Renamed Column From %s To %s\n", old_col, new_col);
            strcpy(head.columnName[i], new_col);
            return i;
        }
    }
    printf("[ERROR]Rename Error No Such Column %s\n", old_col);
    return -1;
}

int Table::getColumnID(const char *name) {
    for (int i = 0; i < head.columnTot; i++)
        if (strcmp(head.columnName[i], name) == 0)
            return i;
    return -1;
}

char *Table::getColumnName(int col) {
    assert(0 <= col && col < head.columnTot);
    return head.columnName[col];
}

int Table::addPrimary(const char *col, const char* pk_name) {
    for (int i = 0; i < head.columnTot; i++) {
        if (strcmp(head.columnName[i], col) == 0) {
            setPrimary(i);
            printf("--Set Primary Key Column %s", col);
            if(pk_name!=nullptr){
                printf(" Primary Key Name %s", pk_name);
                strcpy(head.pkName[i],pk_name);
            }
            printf("\n");
            return i;
        }
    }
    return -1;
}

int Table::dropPrimary_byname(const char *pk_name) {
    int dd = -1;
    for (int i = 0; i < head.columnTot; i++) {
        if (strcmp(head.pkName[i], pk_name) == 0) {
            printf("--Drop Primary Key Column %s\n", head.columnName[i]);
            head.isPrimary &= ~(1 << i);
            --head.primaryCount;
            dd = i;
        }
    }
    if(dd == -1){
        printf("[ERROR]Drop Error No Such Primary Key %s\n", pk_name);
    }
    return dd;
}

int Table::dropForeignByName(const char *fk_name){
    int8_t fkt = head.foreignKeyTot;
    bool fk_flag[fkt];
    for (int8_t i = 0; i < fkt; i++) {
        if (strcmp(head.foreignKeyList[i].name, fk_name) == 0)
            fk_flag[i] = false;
        else
            fk_flag[i] = true;
    }
    head.foreignKeyTot = 0;
    for (int8_t i = 0; i < fkt; i++) {
        if (fk_flag[i]) {
            head.foreignKeyList[head.foreignKeyTot] = head.foreignKeyList[i];
            head.foreignKeyTot++;
        }
    }
}

void Table::dropPrimary() {
    printf("--Drop ALL Primary Key Total %d\n", head.primaryCount);
    head.isPrimary = 0;
    head.primaryCount = 0;
}

void Table::setPrimary(int col) {
    assert((head.notNull >> col) & 1);
    head.isPrimary |= (1 << col);
    head.hasIndex |= (1 << col);
    ++head.primaryCount;
}

void Table::addForeignKeyConstraint(unsigned int col, unsigned int foreignTableId, unsigned int foreignColId, const char* fk_name) {
    assert(head.foreignKeyTot < MAX_FOREIGN_KEY);
    head.foreignKeyList[head.foreignKeyTot].col = col;
    head.foreignKeyList[head.foreignKeyTot].foreign_table_id = foreignTableId;
    head.foreignKeyList[head.foreignKeyTot].foreign_col = foreignColId;
    if(fk_name != nullptr)
        strcpy(head.foreignKeyList[head.foreignKeyTot].name, fk_name);
    head.foreignKeyTot++;
}

void Table::dropForeign() {
    printf("--Drop ALL Foreign Key Total %d\n", head.foreignKeyTot);
    head.foreignKeyTot = 0;
}

void Table::initTempRecord() {
    unsigned int &notNull = *(unsigned int *) buf;
    notNull = 0;
    for (int i = 0; i < head.columnTot; i++) {
        if (head.defaultOffset[i] != -1) {
            switch (head.columnType[i]) {
                case CT_INT:
                case CT_FLOAT:
                case CT_DATE:
                    memcpy(buf + head.columnOffset[i], head.dataArr + head.defaultOffset[i], 4);
                    break;
                case CT_VARCHAR:
                    strcpy(buf + head.columnOffset[i], head.dataArr + head.defaultOffset[i]);
                    break;
                default:
                    assert(false);
            }
            notNull |= (1u << i);
        }
    }
}

void Table::clearTempRecord() {
    if (buf == nullptr) {
        buf = new char[head.recordByte];
        initTempRecord();
    }
}

void Table::setTempRecordNull(int col) {
    if (buf == nullptr) {
        buf = new char[head.recordByte];
        initTempRecord();
    }
    unsigned int &notNull = *(unsigned int *) buf;
    if (notNull & (1u << col)) notNull ^= (1u << col);
}

std::string Table::setTempRecord(int col, const char *data) {
    if (data == nullptr) {
        setTempRecordNull(col);
        return "";
    }
    if (buf == nullptr) {
        buf = new char[head.recordByte];
        initTempRecord();
    }
    unsigned int &notNull = *(unsigned int *) buf;
    switch (head.columnType[col]) {
        case CT_INT:
        case CT_DATE:
        case CT_FLOAT:
            memcpy(buf + head.columnOffset[col], data, 4);
            break;
        case CT_VARCHAR:
            if ((unsigned int) head.columnLen[col] < strlen(data)) {
                printf("[ERROR]Varchar too long\n");
            }
            if (strlen(data) > (unsigned int) head.columnLen[col]) {
                return "[ERROR]Varchar too long";
            }
            strcpy(buf + head.columnOffset[col], data);
            break;
        default:
            assert(0);
    }
    notNull |= (1u << col);
    return "";
}

// return value change. Urgly interface.
// return "" if success.
// return error description otherwise.
std::string Table::insertTempRecord() {
    assert(buf != nullptr);
    if (head.nextAvail == (RID_t) -1) {
        allocPage();
    }
    int rid = head.nextAvail;
    setTempRecord(0, (char *) &head.nextAvail);
    auto error = checkRecord();
    if (!error.empty()) {
        printf("Error occurred when inserting record, aborting...\n");
        return error;
    }
    int pageID = head.nextAvail / PAGE_SIZE;
    int offset = head.nextAvail % PAGE_SIZE;
    int index = BufPageManager::getInstance().getPage(fileID, pageID);
    char *page = (char *)BufPageManager::getInstance().access(index);
    head.nextAvail = *(unsigned int *) (page + offset);
    memcpy(page + offset, buf, head.recordByte);
    BufPageManager::getInstance().markDirty(index);
    inverseFooter(page, offset / head.recordByte);
    for (int i = 0; i < head.columnTot; i++) insertColIndex(rid, i);
    return "";
}

bool Table::isPrimary(int col) {
    return (head.isPrimary & (1 << col)) != 0;
}

bool Table::checkPrimary() {
    if (head.primaryCount == 1) return true;
    int conflictCount = 0;
    int firstPrimary = 1;
    while (!isPrimary(firstPrimary)) {
        ++firstPrimary;
    }
    auto equalFirstIndex = IndexKey(permID, -1, firstPrimary, getFastCmp(-1, firstPrimary),
                                    getIsNull(-1, firstPrimary));
    
    auto rid = colIndex[firstPrimary].lowerBoundEqual(equalFirstIndex);
    while (rid != -1) {
        if (rid == *(int *) (buf + head.columnOffset[0])) {
            // hit the record it self (when updating)
            return true;
        }
        conflictCount = 1;
        for (int col = firstPrimary + 1; col < head.columnTot; ++col) {
            if (!isPrimary(col)) {
                continue;
            }
            char *tmp;
            //char *new_record = getRecordTempPtr();
            switch (head.columnType[col]) {
                case CT_INT:
                case CT_DATE:
                    tmp = select(rid, col);
                    if (*(int *) tmp == *(int *) (buf + head.columnOffset[col])) {
                        ++conflictCount;
                    }
                    free(tmp);
                    break;
                case CT_FLOAT:
                    tmp = select(rid, col);
                    if (*(float *) tmp == *(float *) (buf + head.columnOffset[col])) {
                        ++conflictCount;
                    }
                    free(tmp);
                    break;
                case CT_VARCHAR:
                    tmp = select(rid, col);
                    if (strcmp(tmp, buf + head.columnOffset[col]) == 0) {
                        ++conflictCount;
                    }
                    free(tmp);
                    break;
                default:
                    assert(false);
            }
        }
        if (conflictCount == head.primaryCount - 1) {
            return false;
        }
        rid = colIndex[firstPrimary].nextEqual(equalFirstIndex);
    }
    return true;
}

void Table::clearBuffer() {
    if (buf == nullptr) {
        buf = new char[head.recordByte];
        resetBuffer();
    }
}

void Table::resetBuffer(){
    unsigned int &notNull = *(unsigned int *) buf;
    notNull = 0;
    for (int i = 0; i < head.columnTot; i++) {
        if (head.defaultOffset[i] != -1) {
            switch (head.columnType[i]) {
                case CT_INT:
                case CT_FLOAT:
                case CT_DATE:
                    memcpy(buf + head.columnOffset[i], head.dataArr + head.defaultOffset[i], 4);
                    break;
                case CT_VARCHAR:
                    strcpy(buf + head.columnOffset[i], head.dataArr + head.defaultOffset[i]);
            }
        }
    }
}

std::string Table::checkForeignKeyConstraint() {
    for (int i = 0; i < head.foreignKeyTot; ++i) {
        auto check = head.foreignKeyList[i];
        auto localData = (buf + head.columnOffset[check.col]);
        auto dbms = DBMS::getInstance();
        if (!dbms->valueExistInTable(localData, check)) {
            return "[ERROR]Insert Error Value of column " + std::string(head.columnName[head.foreignKeyList[i].col])
                   + " does not meet foreign key constraint\n";
        }
    }
    return std::string();
}

void Table::printTableDef() {
    printf("---------------------\n");
    printf("ColumnTot %d \n", head.columnTot);
    for (int i = 0; i < head.columnTot; i++) {
        printf("%s", head.columnName[i]);
        switch (head.columnType[i]) {
            case CT_INT:
                printf(" INT(%d)", head.columnLen[i]);
                break;
            case CT_FLOAT:
                printf(" FLOAT");
                break;
            case CT_DATE:
                printf(" DATE");
                break;
            case CT_VARCHAR:
                printf(" VARCHAR(%d)", head.columnLen[i]);
                break;
            default:
                assert(0);
        }
        if (head.notNull & (1 << i)) printf(" NotNull");
        if (head.hasIndex & (1 << i)) printf(" Indexed");
        if (head.isPrimary & (1 << i)) printf(" Primary");
        printf("\n");
    }
    for(int i = 0; i<head.foreignKeyTot;i++){
        printf("Foreigner Key %s from %d to %d.%d\n",
        head.foreignKeyList[i].name, head.foreignKeyList[i].col, head.foreignKeyList[i].foreign_table_id, head.foreignKeyList[i].foreign_col);
    }
    printf("---------------------\n");
}

std::string Table::checkRecord() {
    unsigned int &notNull = *(unsigned int *) buf;
    if ((notNull & head.notNull) != head.notNull) {
        return "[ERROR]Insert Error Get Null in not Null Column\n";
    }
    if(!noCheck){
        if (!checkPrimary()) {
            return "[ERROR]Primary Key Conflict\n";
        }
        auto foreignKeyCheck = checkForeignKeyConstraint();
        if (!foreignKeyCheck.empty()) {
            return foreignKeyCheck;
        }
    }
    return std::string();
}

std::string Table::loadRecordToTemp(RID_t rid, char *page, int offset) {
    UNUSED(rid);
    if (buf == nullptr) {
        buf = new char[head.recordByte];
    }
    char *record = page + offset;
    if (!getFooter(page, offset / head.recordByte)) {
        return "ERROR: RID invalid";
    }
    memcpy(buf, record, (size_t) head.recordByte);
    return "";
}

std::string Table::modifyRecordNull(RID_t rid, int col) {
    int pageID = rid / PAGE_SIZE;
    int offset = rid % PAGE_SIZE;
    int index = BufPageManager::getInstance().getPage(fileID, pageID);
    char *page = (char*)BufPageManager::getInstance().access(index);
    char *record = page + offset;
    std::string err = loadRecordToTemp(rid, page, offset);
    if (!err.empty()) {
        return err;
    }
    assert(col != 0);
    setTempRecordNull(col);
    err = checkRecord();
    if (!err.empty()) {
        return err;
    }
    eraseColIndex(rid, col);
    memcpy(record, buf, head.recordByte);
    BufPageManager::getInstance().markDirty(index);
    insertColIndex(rid, col);
    return "";
}

std::string Table::modifyRecord(RID_t rid, int col, char *data) {
    if (data == nullptr) {
        return modifyRecordNull(rid, col);
    }
    int pageID = rid / PAGE_SIZE;
    int offset = rid % PAGE_SIZE;
    int index = BufPageManager::getInstance().getPage(fileID, pageID);
    char *page = BufPageManager::getInstance().access(index);
    char *record = page + offset;
    std::string err = loadRecordToTemp(rid, page, offset);
    if (!err.empty()) {
        return err;
    }
    assert(col != 0);
    err = setTempRecord(col, data);
    if (!err.empty()) {
        return err;
    }
    err = checkRecord();
    if (!err.empty()) {
        return err;
    }
    eraseColIndex(rid, col);
    memcpy(record, buf, head.recordByte);
    BufPageManager::getInstance().markDirty(index);
    insertColIndex(rid, col);
    return "";
}

RID_t Table::selectIndexLowerBound(int col, const char *data) {
    if (data == nullptr) {
        return selectIndexLowerBoundNull(col);
    }
    assert(hasIndex(col));
    setTempRecord(col, data);
    return colIndex[col].lowerBound(IndexKey(permID, -1, col, getFastCmp(-1, col), getIsNull(-1, col)));
}

RID_t Table::selectIndexLowerBoundEqual(int col, const char *data) {
    if (data == nullptr) {
        return selectIndexLowerBoundNull(col);
    }
    assert(hasIndex(col));
    setTempRecord(col, data);
    return colIndex[col].lowerBoundEqual(IndexKey(permID, -1, col, getFastCmp(-1, col), getIsNull(-1, col)));
}

RID_t Table::selectIndexLowerBoundNull(int col) {
    assert(hasIndex(col));
    return colIndex[col].begin();
}

RID_t Table::selectIndexNext(int col) {
    assert(hasIndex(col));
    return colIndex[col].next();
}

RID_t Table::selectIndexNextEqual(int col) {
    assert(hasIndex(col));
    return colIndex[col].nextEqual(IndexKey(permID, -1, col, getFastCmp(-1, col), getIsNull(-1, col)));
}

RID_t Table::selectIndexUpperBound(int col, const char *data) {
    if (data == nullptr) {
        return selectIndexUpperBoundNull(col);
    }
    assert(hasIndex(col));
    setTempRecord(col, data);
    return colIndex[col].upperBound(IndexKey(permID, -1, col, getFastCmp(-1, col), getIsNull(-1, col)));
}

RID_t Table::selectIndexUpperBoundNull(int col) {
    assert(hasIndex(col));
    return colIndex[col].end();
}

RID_t Table::selectReveredIndexNext(int col) {
    assert(hasIndex(col));
    return colIndex[col].reversedNext();
}

void Table::changeColumn(const char *name, struct column_defs *col_def){
    int8_t id = -1;
    for (int i = 0; i < head.columnTot; i++) {
        if (strcmp(head.columnName[i], name) == 0)
            id = i;
    }
    if (id == -1){
        printf("[ERROR]Change Column Error Column does not exits\n");
        return;
    }
    auto type = (ColumnType) 0;
    switch (col_def->type) {
        case COLUMN_TYPE_INT:
            type = CT_INT;
            break;
        case COLUMN_TYPE_VARCHAR:
            type = CT_VARCHAR;
            break;
        case COLUMN_TYPE_FLOAT:
            type = CT_FLOAT;
            break;
        case COLUMN_TYPE_DATE:
            type = CT_DATE;
            break;
        default:
            assert(0==1);
            break;
    }
    head.columnType[id] = type;
    strcpy(head.columnName[id], col_def->name);
    bool isDefault = (bool) (col_def->flags->flags & COLUMN_FLAG_DEFAULT);
    bool isNotNull= (bool) (col_def->flags->flags & COLUMN_FLAG_NOTNULL);
    if(isNotNull){
        head.notNull |= (1 << id);
    }
    else{
        head.notNull &= ~(1 << id);
    }

    int size = 0;
    char temp_dataArr[MAX_DATA_SIZE];
    memcpy(temp_dataArr, head.dataArr, MAX_DATA_SIZE);
    switch (type) {
        case CT_INT:
            size = 4;
            memcpy(head.dataArr + head.columnOffset[id], &(col_def->flags->default_value->literal_i), 4);
            break;
        case CT_FLOAT:
            size = 4;
            memcpy(head.dataArr + head.columnOffset[id], &(col_def->flags->default_value->literal_f), 4);
            break;
        case CT_DATE:
            size = 4;
            memcpy(head.dataArr + head.columnOffset[id], &(col_def->flags->default_value->literal_i), 4);
            break;
        case CT_VARCHAR:
            size = MAX_DATA_LEN + 1;
            strcpy(head.dataArr + head.columnOffset[id], col_def->flags->default_value->literal_s);
            break;
        default:
            assert(0);
    }
    int diff = size - head.columnLen[id];
    head.recordByte += diff;
    head.recordByte += 4 - head.recordByte % 4;
    head.columnOffset[id] += diff;
    head.columnLen[id] = size;
    if(isDefault){
        head.defaultOffset[id] += diff;
        head.dataArrUsed += diff;
    }
    
    for(int i = id + 1;i<head.columnTot;i++){
        head.columnOffset[i] += diff;
        if(isDefault){
            head.defaultOffset[i] += diff;
            memcpy(head.dataArr+head.defaultOffset[i], temp_dataArr + head.defaultOffset[i]-diff, size);
        }
    }
}