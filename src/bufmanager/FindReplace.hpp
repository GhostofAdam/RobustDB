#ifndef __BUF_SEARCH_H__
#define __BUF_SEARCH_H__
#ifdef __cplusplus
extern "C" {
#endif
#include "../utils/MultiList.hpp"
#include "../utils/pagedef.hpp"

class FindReplace {
private:
    MultiList *list;
public:
    FindReplace() {
        list = new MultiList(BUF_CAPACITY, 1);
        for (int i = 0; i < BUF_CAPACITY; i++) {
            list->insert(0, i);
        }
    }

    ~FindReplace() {
        delete list;
    }

    void free(int index) {
        list->insert(0, index);
    }

    void access(int index) {
        list->insert(0, index);
    }

    int find() {
        int index = list->getFirst(0);
        list->erase(index);
        list->insert(0, index);
        return index;
    }

};
#ifdef __cplusplus
}
#endif
#endif
