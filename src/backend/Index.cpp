#include "Index.hpp"


string Index::getFilename(int tab, int col) {
    ostringstream stm;
    stm << tab << "." << col << ".id";
    return stm.str();
}

void Index::clear() {
    this->list.clear();
    this->iter = this->list.begin();
}

void Index::load(int tab, int col) {
    ifstream stm(getFilename(tab, col).c_str());
    this->list.restore(stm);
} 

void Index::store(int tab, int col) {
    printf("storing index %d %d\n",tab,col);
    ofstream stm(getFilename(tab, col).c_str());
    this->list.dump(stm);
}

void Index::drop(int tab, int col) {
    remove(getFilename(tab, col).c_str());
}

void Index::erase(const IndexKey &key) {
    assert(this->list.erase(key) == 1);
}

void Index::insert(const IndexKey &key) {
    assert(this->list.find(key) == this->list.end());
    this->list.insert(key);
}

int Index::begin() {
    if (list.begin() == list.end()) return -1;    // 没有元素
    iter = list.begin();
    return iter->getRid();
}

int Index::end() {
    if (list.begin() == list.begin()) return -1;        // 没有元素
    iter = list.end();
    iter--;
    return iter->getRid();
}

int Index::lowerBound(const IndexKey &key) {
    iter = list.lower_bound(key);
    if (iter == list.end()) return -1;
    return iter->getRid();
}

int Index::upperBound(const IndexKey &key) {
    iter = list.upper_bound(key);
    if (iter == list.begin()) return -1;
    iter--;
    return iter->getRid();
}

int Index::lowerBoundEqual(const IndexKey &key) {
    iter = list.lower_bound(key);
    if (iter == list.end()){
        return -1;  
    }
    if (iter->getFastCmp() == key.getFastCmp()){
        return iter->getRid();
    }
    return -1;
}

int Index::next() {
    if (iter == list.end()) return -1;
    iter++;
    if (iter == list.end()) return -1;
    return iter->getRid();
}

int Index::nextEqual(const IndexKey &key){
    if (iter == list.end()) return -1;
    iter++;
    if (iter == list.end()) return -1;
    if (iter->getFastCmp() == key.getFastCmp()) return iter->getRid();
    return -1;
}

int Index::reversedNext() {
    if (iter == list.begin()) return -1;
    iter--;
    return iter->getRid();
}