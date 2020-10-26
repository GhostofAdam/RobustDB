#include <cstring>
#include <string>

#include "Table.hpp"

Table::Table() {
    this->ready = false;
}

Table::~Table() {
    if (this->ready) close();
}

void Table::create(string tableName) {
    assert(!this->ready);
    this->tableName = tableName;
    BufPageManager::getFileManager().createFile(tableName);
    fileID = BufPageManager::getFileManager().openFile(tableName);
    permID = BufPageManager::getFileManager().getFilePermID(fileID);
    BufPageManager::getInstance().allocPage(fileID, 0);
    RegisterManager::getInstance().checkIn(permID, this);
    this->ready = true;
    this->buf = nullptr;
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

void Table::open(string tableName) {
    assert(!this->ready);
    this->tableName = tableName;
    fileID = BufPageManager::getFileManager().openFile(tableName);
    permID = BufPageManager::getFileManager().getFilePermID(fileID);
    RegisterManager::getInstance().checkIn(permID, this);
    int index = BufPageManager::getInstance().getPage(fileID, 0);
    memcpy(&head, BufPageManager::getInstance().access(index), sizeof(TableHead));
    this->ready = true;
    this->buf = nullptr;
}

void Table::close() {
    assert(this->ready);
    storeIndex();
    int index = BufPageManager::getInstance().getPage(fileID, 0);
    memcpy(BufPageManager::getInstance().access(index), &head, sizeof(TableHead));
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

void Table::storeIndex() {
    for (int i = 0; i < head.columnTot; i++)
        if (head.hasIndex & (1 << i)) {
            colIndex[i].store(permID, i);
        }
}

void Table::dropIndex() {
    for (int i = 0; i < head.columnTot; i++)
        if (head.hasIndex & (1 << i)) {
            colIndex[i].drop(permID, i);
        }
}