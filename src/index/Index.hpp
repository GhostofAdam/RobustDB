#include "common.hpp"
#include "../stx/btree_set.hpp"
#include "../backend/RegisterManager.hpp"

class IndexKey {
    int rid;    // record id
    int permID;
    int fastCmp;
    int8_t col;
    bool isNull;

public:
    IndexKey() = default;
    IndexKey(int permID, int rid, int col, int fastCmp, int isNull) {
        this->rid = rid;
        this->permID =permID;
        this->fastCmp = fastCmp;
        this->col = col;
        this->isNull = isNull;
    }
    int getRid() const { return this->rid; }
    int getPermID() const { return this->permID; }
    int getCol() const { return this->col; }
    int getFastCmp() const { return this->fastCmp; }
    bool getIsNull() const { return this->isNull; }
    friend bool operator<(const IndexKey &a, const IndexKey &b);
};

class Index {
private:
    stx::btree_set<IndexKey> list;      // 一个索引对应的b+ tree
    stx::btree_set<IndexKey>::iterator iter;    
    string getFilename(int tab, int col);

public:
    void clear();   // 清空树
    void load(int table, int col);    // 加载对应表格对应列
    void store(int table, int col);   // 存储对应表格对应列
    void drop(int table, int col);    // 删除对应表格对应列
    void erase(const IndexKey &key);    // 删除指定值
    void insert(const IndexKey &key);   // 插入行
    int begin();    // 获取首元素rid
    int end();      // 获取末元素rid
    int lowerBound(const IndexKey &key);    // 第一个大于等于key的元素,若不存在,end()
    int upperBound(const IndexKey &key);    // 第一个大于key的元素,若不存在,end()
    int lowerBoundEqual(const IndexKey &key);
    int next();     // 迭代器的下一个元素,不存在返回-1
    int nextEqual(const IndexKey &key);     // 若迭代器下一个元素等于key,返回rid;否则返回-1
    int reversedNext();     // 迭代器的前一个元素,不存在返回-1
};