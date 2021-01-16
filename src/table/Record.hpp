#ifndef __RECORD_HPP__
#define __RECORD_HPP__
#include "Table.hpp"
class Record{
public:
    static std::string loadRecordToTemp(Table* tb, RID_t rid, char *page, int offset){
        UNUSED(rid);
        if (tb->buf == nullptr) {
            tb->buf = new char[tb->head.recordByte];
        }
        char *record = page + offset;
        if (!tb->getFooter(page, offset / tb->head.recordByte)) {
            return "ERROR: RID invalid";
        }
        memcpy(tb->buf, record, (size_t) tb->head.recordByte);
        return "";
    }

    static std::string modifyRecord(Table* tb, RID_t rid, int col, char *data = nullptr){
        int pageID = rid / PAGE_SIZE;
        int offset = rid % PAGE_SIZE;
        int index = BufPageManager::getInstance().getPage(tb->fileID, pageID);
        char *page = BufPageManager::getInstance().access(index);
        char *record = page + offset;
        std::string err = loadRecordToTemp(tb, rid, page, offset);
        if (!err.empty()) {
            return err;
        }
        err = tb->insert2Buffer(col, data);
        if (!err.empty()) {
            return err;
        }
        err = tb->checkRecord();
        if (!err.empty()) {
            return err;
        }
        TableIndex::eraseColIndex(tb, rid, col);
        memcpy(record, tb->buf, tb->head.recordByte);
        BufPageManager::getInstance().markDirty(index);
        TableIndex::insertColIndex(tb, rid, col);
        return "";
    }
};
#endif