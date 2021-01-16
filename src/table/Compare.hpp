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
};

#endif /* PARSER_H_ */
