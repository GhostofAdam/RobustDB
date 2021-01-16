#ifndef __TABLEINDEX_HPP__
#define __TABLEINDEX_HPP__
#include "Table.hpp"

class TableIndex{
public:
    static void eraseColIndex(Table* tb, RID_t rid, int col){
        if (tb->hasIndex(col)) {
            tb->colIndex[col].erase(IndexKey(tb->permID, rid, col, tb->getFastCmp(rid, col), tb->getIsNull(rid, col)));
        }
    }
    static void insertColIndex(Table* tb, RID_t rid, int col){
        if (tb->hasIndex(col)) {
            tb->colIndex[col].insert(IndexKey(tb->permID, rid, col, tb->getFastCmp(rid, col), tb->getIsNull(rid, col)));
        }
    }
    static void createIndex(Table* tb, int col, char *name = nullptr){
        if(name)
            strcpy(tb->head.indexName[col], name);
        tb->head.hasIndex |= 1 << col;
    }
    static void dropIndex(Table* tb, char *name){
        for(int col = 0; col<tb->head.columnTot;col++){
            if(strcmp(tb->head.indexName[col],name) == 0){
                tb->head.hasIndex &= ~(1 << col);
                tb->colIndex[col].drop(tb->permID, col);
            }
        }
    }
    static void loadIndex(Table* tb) {
        for (int i = 0; i < tb->head.columnTot; i++)
            if (tb->head.hasIndex & (1 << i))   // bitmap
                tb->colIndex[i].load(tb->permID, i);    // 从文件加载对应列
    }

    static void storeIndex(Table* tb) {
        for (int i = 0; i < tb->head.columnTot; i++)
            if (tb->head.hasIndex & (1 << i)) 
                tb->colIndex[i].store(tb->permID, i);
    }

    static void dropIndex(Table* tb) {
        for (int i = 0; i < tb->head.columnTot; i++)
            if (tb->head.hasIndex & (1 << i))
                tb->colIndex[i].drop(tb->permID, i);
    }
    
    static RID_t selectIndexLowerBoundEqual(Table* tb, int col, const char *data){
        if (data == nullptr) {
            return selectIndexLowerBoundNull(tb, col);
        }
        tb->insert2Buffer(col, data);
        return tb->colIndex[col].lowerBoundEqual(IndexKey(tb->permID, -1, col, tb->getFastCmp(-1, col), tb->getIsNull(-1, col)));
    }
    static RID_t selectIndexLowerBound(Table* tb, int col, const char *data){
        if (data == nullptr) {
            return selectIndexLowerBoundNull(tb, col);
        }
        tb->insert2Buffer(col, data);
        return tb->colIndex[col].lowerBound(IndexKey(tb->permID, -1, col, tb->getFastCmp(-1, col), tb->getIsNull(-1, col)));
    }
    static RID_t selectIndexLowerBoundNull(Table* tb, int col){
        return tb->colIndex[col].begin();
    }
    static RID_t selectIndexNext(Table* tb, int col){
        return tb->colIndex[col].next();
    }
    static RID_t selectIndexNextEqual(Table* tb, int col){
        return tb->colIndex[col].nextEqual(IndexKey(tb->permID, -1, col, tb->getFastCmp(-1, col), tb->getIsNull(-1, col)));
    }
    static RID_t selectIndexUpperBound(Table* tb, int col, const char *data){
        if (data == nullptr) {
            return selectIndexUpperBoundNull(tb, col);
        }
        tb->insert2Buffer(col, data);
        return tb->colIndex[col].upperBound(IndexKey(tb->permID, -1, col, tb->getFastCmp(-1, col), tb->getIsNull(-1, col)));
    }
    static RID_t selectIndexUpperBoundNull(Table* tb, int col){
        return tb->colIndex[col].end();
    }
    static RID_t selectReveredIndexNext(Table* tb, int col){
        return tb->colIndex[col].reversedNext();
    }
    static RID_t getNext(Table* tb, RID_t rid) {
        int page_id, id, n;
        n = (PAGE_SIZE - PAGE_FOOTER_SIZE) / tb->head.recordByte;
        n = (n < MAX_REC_PER_PAGE) ? n : MAX_REC_PER_PAGE;
        if (rid == (RID_t) -1) {
            page_id = 0;
            id = n - 1;
        } else {
            page_id = rid / PAGE_SIZE;
            id = (rid % PAGE_SIZE) / tb->head.recordByte;
        }
        int index = BufPageManager::getInstance().getPage(tb->fileID, page_id);
        char *page =    BufPageManager::getInstance().access(index);

        while (true) {
            id++;
            if (id == n) {
                page_id++;
                if (page_id >= tb->head.pageTot) return (RID_t) -1;
                index = BufPageManager::getInstance().getPage(tb->fileID, page_id);
                page = BufPageManager::getInstance().access(index);
                id = 0;
            }
            if (tb->getFooter(page, id)) return (RID_t) page_id * PAGE_SIZE + id * tb->head.recordByte;
        }
    }
};
#endif