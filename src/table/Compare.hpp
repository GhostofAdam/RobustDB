/*
 * parser.h
 *
 *  Created on: 2014年12月7日
 *      Author: lql
 */

#ifndef PARSER_H_
#define PARSER_H_

#include "../utils/common.hpp"
#ifdef __cplusplus
extern "C" {
#endif
#include <cstring>
#ifdef __cplusplus
}
#endif
#include "../utils/pagedef.hpp"
#include "../parser/type_def.hpp"
using namespace std;
#define LL_TYPE 0
#define DB_TYPE 1
#define ST_TYPE 2
#define L 0
#define G 3
#define LE 1
#define GE 2
#define E 4
#define IS 5
#define UNI 0
#define UNUNI 1
#define N 0
#define UN 1
#define ALL 0
#define RANGE 1
#define ISNULL 2
#define NOTHING 3
enum ColumnType {
    CT_INT, CT_VARCHAR, CT_FLOAT, CT_DATE
};
enum OpType {
    OP_EQ, OP_GE, OP_LE, OP_GT, OP_LT
};
enum RelType {
    RE_OR, RE_AND
};

class Compare{
    public:
    template<class Key>
    static int keyu(uchar* a, uchar* b) {
        Key c, d;
        memcpy(&c, a, sizeof(Key));
        memcpy(&d, b, sizeof(Key));
        return ((c < d) ? -1 : ((c > d) ? 1 : 0));
    }
    template<class Key>
    static int keyn(uchar* a, uchar* b) {
        int res = keyu<Key>(a, b);
        return (res != 0) ? res : keyu<ll>(a + sizeof(Key), b + sizeof(Key));
    }
    static int su(uchar* a, uchar* b) {
        int res = strcmp((char*)a, (char*)b);
        return (res > 0) ? 1 : ((res < 0) ? -1 : 0);
    }
    static int sn(uchar* a, uchar* b) {
        int tmp = 0;
        int res = strcmp((char*)a, (char*)b);
        res = (res > 0) ? 1 : ((res < 0) ? -1 : 0);
        return (res != 0) ? res : keyu<ll>(a + tmp, b + tmp);
    }
    static bool compareFloat(float x, OpType op, float y) {
        switch (op) {
            case OP_EQ:
                return x == y;
            case OP_GE:
                return x >= y;
            case OP_LE:
                return x <= y;
            case OP_GT:
                return x > y;
            case OP_LT:
                return x < y;
            default:
                break;
        }
    }

    static bool compareInt(int x, OpType op, int y) {
        switch (op) {
            case OP_EQ:
                return x == y;
            case OP_GE:
                return x >= y;
            case OP_LE:
                return x <= y;
            case OP_GT:
                return x > y;
            case OP_LT:
                return x < y;
            default:
                break;
        }
    }

    static bool compareVarchar(const char *x, OpType op, const char *y) {
        switch (op) {
            case OP_EQ:
                return strcmp(x, y) == 0;
            case OP_GE:
                return strcmp(x, y) >= 0;
            case OP_LE:
                return strcmp(x, y) <= 0;
            case OP_GT:
                return strcmp(x, y) > 0;
            case OP_LT:
                return strcmp(x, y) < 0;
            default:
                break;
        }
    }

    static int sgn(int x) {
        if (x == 0) return 0;
        if (x > 0) return 1;
        return -1;
    }

    static int compareIntSgn(int x, int y) {
        return sgn(x - y);
    }

    static int compareVarcharSgn(char *x, char *y) {
        return sgn(strcmp(x, y));
    }

    static int floatSgn(float x) {
        if (x == 0) return 0;
        if (x > 0) return 1;
        return -1;
    }

    static int compareFloatSgn(float x, float y) {
        return floatSgn(x - y);
    }

    static std::string opTypeToString(OpType op) {
        switch (op) {
            case OP_EQ:
                return "==";
            case OP_GE:
                return ">=";
            case OP_LE:
                return "<=";
            case OP_GT:
                return ">";
            case OP_LT:
                return "<";
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
    ListNode* mergeKLists(vector<ListNode*>& lists) {
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

    void binaryMerge(vector<ListNode*>& lists) {
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
    ListNode*merge(ListNode* head1, ListNode* head2) {
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

    vector<int> findSubstring(string s, vector<string>& words) {
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

    string multiply(string num1, string num2) {
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

    string addStrings(string &num1, string &num2) {
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

};
struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};


#endif /* PARSER_H_ */
