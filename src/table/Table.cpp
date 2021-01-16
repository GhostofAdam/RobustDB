
#include "PrimaryKey.hpp"
#include "ForeignerKey.hpp"
#include "TableIndex.hpp"
#include "Record.hpp"
#include "../dbms/DBMS.hpp"
class DBMS;
bool operator<(const IndexKey &a, IndexKey &b) {
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
            break;
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
            break;
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
    this->tableName = tableName;
    auto bpm = BufPageManager::getInstance();
    auto fm = BufPageManager::getFileManager();
    auto rm = RegisterManager::getInstance();
    bpm.createFile(tableName); // 创建table对应文件
    this->fileID = fm.openFile(tableName);  
    this->permID = fm.getFilePermID(this->fileID);
    bpm.allocPage(this->fileID, 0); // 为文件首页获取一个缓存中的页面
    rm.checkIn(permID, this);
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
    PrimaryKey::setPrimary(this,0);
    for (auto &col: colIndex) {
        col.clear();
    }
}

void Table::open(const char* tableName) {
    this->tableName = tableName;
    auto bpm = BufPageManager::getInstance();
    auto fm = BufPageManager::getFileManager();
    auto rm = RegisterManager::getInstance();
    this->fileID = fm.openFile(tableName);
    this->permID = fm.getFilePermID(fileID);
    rm.checkIn(permID, this);
    int index = bpm.getPage(fileID, 0);   // 为文件首页在缓存中找到对应缓存页面
    memcpy(&(this->head), bpm.access(index), sizeof(TableHead));
    this->ready = true;
    this->buf = nullptr;
    for (auto &col: this->colIndex)
        col.clear();
    TableIndex::loadIndex(this);    // 加载各列信息
}

void Table::close() {
    TableIndex::storeIndex(this);
    auto bpm = BufPageManager::getInstance();
    auto fm = BufPageManager::getFileManager();
    auto rm = RegisterManager::getInstance();

    int index = bpm.getPage(fileID, 0);
    memcpy(bpm.access(index), &(this->head), sizeof(this->head));

    bpm.markDirty(index);
    rm.checkOut(permID);
    bpm.closeFile(fileID);
    fm.closeFile(fileID);
    this->ready = false;
    if (buf) {
        delete[] buf;
        buf = 0;
    }
}

void Table::drop() {
    TableIndex::dropIndex(this);    // 删除所有列
    auto bpm = BufPageManager::getInstance();
    auto fm = BufPageManager::getFileManager();
    auto rm = RegisterManager::getInstance();
    rm.checkOut(permID);
    bpm.closeFile(fileID, false);
    fm.closeFile(fileID);
    this->ready = false;
}

void Table::allocPage() {
    auto bpm = BufPageManager::getInstance();
    auto index = bpm.allocPage(fileID, head.pageTot);
    auto buf = bpm.access(index);
    auto n = (PAGE_SIZE - PAGE_FOOTER_SIZE) / head.recordByte;
    n = (n < MAX_REC_PER_PAGE) ? n : MAX_REC_PER_PAGE;
    for (int i = 0, p = 0; i < n; i++, p += head.recordByte) {
        unsigned int &ptr = *(unsigned int *) (buf + p);
        ptr = head.nextAvail;
        head.nextAvail = (unsigned int) head.pageTot * PAGE_SIZE + p;
    }
    memset(buf + PAGE_SIZE - PAGE_FOOTER_SIZE, 0, PAGE_FOOTER_SIZE);
    bpm.markDirty(index);
    head.pageTot++;
}


std::string Table::getTableName() {
    return tableName;
}

char *Table::getRecordTempPtr(RID_t rid) {
    int pageID = rid / PAGE_SIZE;
    int offset = rid % PAGE_SIZE;
    auto bpm = BufPageManager::getInstance();
    auto index = bpm.getPage(fileID, pageID);
    auto page = bpm.access(index);
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
            break;
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
            break;
    }
    delete[] p;
    return res;
}

bool Table::getIsNull(RID_t rid, int col) {
    char *p = select(rid, col);
    delete[] p;
    return p == nullptr;
}

void Table::dropRecord(RID_t rid) {
    int pageID = rid / PAGE_SIZE;
    int offset = rid % PAGE_SIZE;
    for (int i = 0; i < head.columnTot; i++) {
        if (head.hasIndex & (1 << i)) TableIndex::eraseColIndex(this, rid, i);
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
            break;
    }

    printf("--Added Column: %s\n", name);
    return id;
}

bool Table::insert2Buffer(int col, const char *data){
    if (buf == nullptr) {
        buf = new char[head.recordByte];
        resetBuffer();
    }
    unsigned int &notNull = *(unsigned int *) buf;
    if(data == nullptr){
        if (notNull & (1u << col)) notNull ^= (1u << col);
        return true;
    }
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
            break;
    }
    notNull |= (1u << col);
    return true;
}

bool Table::insert2Record(){
    if (head.nextAvail == (RID_t) -1) {
        allocPage();
    }
    int rid = head.nextAvail;
    insert2Buffer(0, (char *) &head.nextAvail);
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
    for (int i = 0; i < head.columnTot; i++) TableIndex::insertColIndex(this, rid, i);
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
int checkColumn(const char *name){
    if(name!)
        return -1;
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
int Table::dropColumnifNull(const char *name) {
    if(name!)
        return -1;
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
    return head.columnName[col];
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
    if (buf == nullptr) {
        buf = new char[head.recordByte];
        resetBuffer();
    }
    for (int i = 0; i < head.columnTot; i++) {
        if (head.defaultOffset[i] != -1) {
            switch (head.columnType[i]) {
                case CT_INT:
                    memcpy(buf + head.columnOffset[i], head.dataArr + head.defaultOffset[i], 4);
                    break;
                case CT_FLOAT:
                    memcpy(buf + head.columnOffset[i], head.dataArr + head.defaultOffset[i], 4);
                    break;
                case CT_DATE:
                    memcpy(buf + head.columnOffset[i], head.dataArr + head.defaultOffset[i], 4);
                    break;
                case CT_VARCHAR:
                    strcpy(buf + head.columnOffset[i], head.dataArr + head.defaultOffset[i]);
            }
        }
    }
}

void Table::printTableDef() {
    printf("---------------------\n");
    printf("ColumnTot %d \n", head.columnTot);
    for (int i = 0; i < head.columnTot; i++) {
        printf("%s", head.columnName[i]);
        switch (head.columnType[i]) {
            case CT_INT:
                printf(" ");
                printf("INT(%d)", head.columnLen[i]);
                break;
            case CT_FLOAT:
                printf(" ");
                printf("FLOAT");
                break;
            case CT_DATE:
                printf(" ");
                printf("DATE");
                break;
            case CT_VARCHAR:
                printf(" ");
                printf("VARCHAR(%d)", head.columnLen[i]);
                break;
            default:
                break;
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
    if (buf == nullptr) {
        buf = new char[head.recordByte];
        resetBuffer();
    }
    unsigned int &notNull = *(unsigned int *) buf;
    if ((notNull & head.notNull) != head.notNull) {
        return "[ERROR]Insert Error Get Null in not Null Column\n";
    }
    if(!noCheck){
        if (!PrimaryKey::checkPrimary(this)) {
            return "[ERROR]Primary Key Conflict\n";
        }
        auto foreignKeyCheck = checkForeignKeyConstraint();
        if (!foreignKeyCheck.empty()) {
            return foreignKeyCheck;
        }
    }
    return std::string();
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
            break;
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




std::string Table::checkForeignKeyConstraint(){
    if (buf == nullptr) {
        buf = new char[head.recordByte];
        resetBuffer();
    }
    for (int i = 0; i < this->head.foreignKeyTot; ++i) {
        auto check = this->head.foreignKeyList[i];
        auto localData = (this->buf + this->head.columnOffset[check.col]);
        auto dbms = DBMS::getInstance();
        if (!dbms->valueExistInTable(localData, check)) {
            return "[ERROR]Insert Error Value of column " + std::string(this->head.columnName[this->head.foreignKeyList[i].col])
                + " does not meet foreign key constraint\n";
        }
    }
    return std::string();
}

void Table::checkConstraint(){
    int size = 0, col = 0;
    char temp_dataArr[MAX_DATA_SIZE];
    memcpy(temp_dataArr, head.dataArr, MAX_DATA_SIZE);
    switch (type) {
        case CT_INT:
            size = 4;
            memcpy(head.dataArr + head.columnOffset[col], &(col_def->flags->default_value->literal_i), 4);
            break;
        case CT_FLOAT:
            size = 4;
            memcpy(head.dataArr + head.columnOffset[col], &(col_def->flags->default_value->literal_f), 4);
            break;
        case CT_DATE:
            size = 4;
            memcpy(head.dataArr + head.columnOffset[col], &(col_def->flags->default_value->literal_i), 4);
            break;
        case CT_VARCHAR:
            size = MAX_DATA_LEN + 1;
            strcpy(head.dataArr + head.columnOffset[col], col_def->flags->default_value->literal_s);
            break;
        default:
            break;
    }
    auto check = this->head.foreignKeyList[col];
    auto localData = (this->buf + this->head.columnOffset[check.col]);
    auto dbms = DBMS::getInstance();
    int size = this->head.columnOffset[check.col]
    int diff = size - head.columnLen[col];
    head.recordByte += diff;
    head.recordByte += 4 - head.recordByte % 4;
    head.columnOffset[col] += diff;
    head.columnLen[col] = size;

    char temp_dataArr[MAX_DATA_SIZE];
    memcpy(temp_dataArr, head.dataArr, MAX_DATA_SIZE);
    switch (type) {
        case CT_INT:
            size = 4;
            memcpy(head.dataArr + head.columnOffset[col], &(col_def->flags->default_value->literal_i), 4);
            break;
        case CT_FLOAT:
            size = 4;
            memcpy(head.dataArr + head.columnOffset[col], &(col_def->flags->default_value->literal_f), 4);
            break;
        case CT_DATE:
            size = 4;
            memcpy(head.dataArr + head.columnOffset[col], &(col_def->flags->default_value->literal_i), 4);
            break;
        case CT_VARCHAR:
            size = MAX_DATA_LEN + 1;
            strcpy(head.dataArr + head.columnOffset[col], col_def->flags->default_value->literal_s);
            break;
        default:
            break;
    }
    auto check = this->head.foreignKeyList[col];
    auto localData = (this->buf + this->head.columnOffset[check.col]);
    auto dbms = DBMS::getInstance();
    int size = this->head.columnOffset[check.col]
    int diff = size - head.columnLen[col];
    head.recordByte += diff;
    head.recordByte += 4 - head.recordByte % 4;
    head.columnOffset[col] += diff;
    head.columnLen[col] = size;
}

void Table::insertRecord2Buf(RID_t rid){
    auto type = (ColumnType) 0;
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
            break;
    }
}
ListNode* Table::visitLists(vector<ListNode*>& lists){
    const int size = lists.size();

    for (int col = 0; col < size; col+=2) {
        if (col+1 >= size) {
            lists[col/2] = lists[col];
            auto check = this->head.foreignKeyList[col];
            auto localData = (this->buf + this->head.columnOffset[check.col]);
            auto dbms = DBMS::getInstance();
            int size = this->head.columnOffset[check.col]
            int diff = size - head.columnLen[col];
            head.recordByte += diff;
            head.recordByte += 4 - head.recordByte % 4;
            head.columnOffset[col] += diff;
            head.columnLen[col] = size;
        }
        lists[col/2] = merge(lists[col], lists[col+1]);
    }

    int col = size / 2;
    while (col--) {
        lists.pop_back();
        auto check = this->head.foreignKeyList[col];
        auto localData = (this->buf + this->head.columnOffset[check.col]);
        auto dbms = DBMS::getInstance();
        int size = this->head.columnOffset[check.col]
        int diff = size - head.columnLen[col];
        head.recordByte += diff;
        head.recordByte += 4 - head.recordByte % 4;
        head.columnOffset[col] += diff;
        head.columnLen[col] = size;

    }
}
ListNode* Table::mergeKLists(vector<ListNode*>& lists) {
    if (lists.empty()) return nullptr;

    while (lists.size() != 1) {
        binaryMerge(lists);
        auto type = (ColumnType) 0;
        switch (type) {
        case CT_INT:
            size = 4;
            visitLists(lists);
            break;
        case CT_FLOAT:
            size = 4;
            visitLists(lists);
            break;
        case CT_DATE:
            size = 4;
            visitLists(lists);
            break;
        case CT_VARCHAR:
            size = MAX_DATA_LEN + 1;
            visitLists(lists);
            break;
        default:
            break;
    }
    }

    return lists[0];
}

void Table::binaryMerge(vector<ListNode*>& lists) {
    const int size = lists.size();

    for (int i = 0; i < size; i+=2) {
        if (i+1 >= size) {
            lists[i/2] = lists[i];
            break;
        }
        lists[i/2] = merge(lists[i], lists[i+1]);
    }

    int count = size / 2;
    while (count--) {
        lists.pop_back();
    }
}
ListNode* Table::merge(ListNode* head1, ListNode* head2) {
    ListNode mergeHead;
    ListNode* mergePtr  = &mergeHead;
    while (head1 && head2) {
        if (head1->val < head2->val) {
            mergePtr->next = head1;
            head1 = head1->next;
        }
        else {
            mergePtr->next = head2;
            head2 = head2->next;
        }
        mergePtr = mergePtr->next;
    }
    if (head1) mergePtr->next = head1;
    else       mergePtr->next = head2;
    return mergeHead.next;
}

vector<int> Table::findSubstring(string s, vector<string>& words) {
    if (s == "" || words.size() == 0) return {};
    unordered_map<string, int>hash;
    vector<int>ans; ans.clear();
    int modelen = words[0].length(), len = s.length();
    for (auto s: words) hash[s] ++;
    auto tmp = hash;
    string buff = "";
    for (int i = 0; i < len; i ++) {
        if (len - i + 1 < modelen * words.size()) break;
        tmp = hash;
        buff = s.substr(i, modelen);
        int j = i, count = 0;
        while (tmp[buff] > 0) {
                -- tmp[buff];
                count ++;
                j += modelen;
                buff = s.substr(j, modelen);
                 auto type = (ColumnType) 0;
            switch (type) {
            case CT_INT:
                size = 4;
                visitLists(lists);
                break;
            case CT_FLOAT:
                size = 4;
                visitLists(lists);
                break;
            case CT_DATE:
                size = 4;
                visitLists(lists);
                break;
            case CT_VARCHAR:
                size = MAX_DATA_LEN + 1;
                visitLists(lists);
                break;
            default:
                break;
            }
        if (count == words.size()) {
            ans.push_back(i);
        }
    }
    return ans;
    }
}

string Table::multiply(string num1, string num2) {
    if (num1 == "0" || num2 == "0") {
        return "0";
    }
    string ans = "0";
    int m = num1.size(), n = num2.size();
    for (int i = n - 1; i >= 0; i--) {
        string curr;
        int add = 0;
        for (int j = n - 1; j > i; j--) {
            curr.push_back(0);
        }
        int y = num2.at(i) - '0';
        for (int j = m - 1; j >= 0; j--) {
            int x = num1.at(j) - '0';
            int product = x * y + add;
            curr.push_back(product % 10);
            add = product / 10;
        }
        while (add != 0) {
            curr.push_back(add % 10);
            add /= 10;
        }
        reverse(curr.begin(), curr.end());
        for (auto &c : curr) {
            c += '0';
        }
        ans = addStrings(ans, curr);
    }
    return ans;
}

string Table::addStrings(string &num1, string &num2) {
    int i = num1.size() - 1, j = num2.size() - 1, add = 0;
    string ans;
    while (i >= 0 || j >= 0 || add != 0) {
        int x = i >= 0 ? num1.at(i) - '0' : 0;
        int y = j >= 0 ? num2.at(j) - '0' : 0;
        int result = x + y + add;
        ans.push_back(result % 10);
        add = result / 10;
        i--;
        j--;
    }
    reverse(ans.begin(), ans.end());
    for (auto &c: ans) {
        c += '0';
    }
    return ans;
}