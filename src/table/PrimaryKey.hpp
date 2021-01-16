#ifndef __PK_HPP__
#define __PK_HPP__
#include "Table.hpp"

class PrimaryKey{
public:
    static int addPrimary(Table* tb, const char *col, const char* pk_name = nullptr){
        for (int i = 0; i < tb->head.columnTot; i++) {
            if (strcmp(tb->head.columnName[i], col) == 0) {
                setPrimary(tb, i);
                printf("--Set Primary Key Column %s", col);
                if(pk_name!=nullptr){
                    printf(" Primary Key Name %s", pk_name);
                    strcpy(tb->head.pkName[i],pk_name);
                }
                printf("\n");
                return i;
            }
        }
        return -1;
    }
    static int dropPrimaryByName(Table* tb, const char *pk_name){
        int dd = -1;
        for (int i = 0; i < tb->head.columnTot; i++) {
            if (strcmp(tb->head.pkName[i], pk_name) == 0) {
                printf("--Drop Primary Key Column %s\n", tb->head.columnName[i]);
                tb->head.isPrimary &= ~(1 << i);
                --(tb->head.primaryCount);
                dd = i;
            }
        }
        if(dd == -1){
            printf("[ERROR]Drop Error No Such Primary Key %s\n", pk_name);
        }
        return dd;
    }
    static void dropPrimary(Table* tb){
        printf("--Drop ALL Primary Key Total %d\n", tb->head.primaryCount);
        tb->head.isPrimary = 0;
        tb->head.primaryCount = 0;
    }
    static void setPrimary(Table* tb, int col){
        tb->head.isPrimary |= (1 << col);
        tb->head.hasIndex |= (1 << col);
        ++(tb->head.primaryCount);
    }
    static bool isPrimary(Table* tb, int col){
        return (tb->head.isPrimary & (1 << col)) != 0;
    }
    static bool checkPrimary(Table* tb){
        if (tb->head.primaryCount == 1) return true;
        int conflictCount = 0;
        int firstPrimary = 1;
        while (!isPrimary(tb, firstPrimary)) {
            ++firstPrimary;
        }
        auto equalFirstIndex = IndexKey(tb->permID, -1, firstPrimary, tb->getFastCmp(-1, firstPrimary),
                                        tb->getIsNull(-1, firstPrimary));
        
        auto rid = tb->colIndex[firstPrimary].lowerBoundEqual(equalFirstIndex);
        while (rid != -1) {
            if (rid == *(int *) (tb->buf + tb->head.columnOffset[0])) {
                // hit the record it self (when updating)
                return true;
            }
            conflictCount = 1;
            for (int col = firstPrimary + 1; col < tb->head.columnTot; ++col) {
                if (!isPrimary(tb, col)) {
                    continue;
                }
                char *tmp;
                //char *new_record = getRecordTempPtr();
                switch (tb->head.columnType[col]) {
                    case CT_INT:
                    case CT_DATE:
                        tmp = tb->select(rid, col);
                        if (*(int *) tmp == *(int *) (tb->buf + tb->head.columnOffset[col])) {
                            ++conflictCount;
                        }
                        free(tmp);
                        break;
                    case CT_FLOAT:
                        tmp = tb->select(rid, col);
                        if (*(float *) tmp == *(float *) (tb->buf + tb->head.columnOffset[col])) {
                            ++conflictCount;
                        }
                        free(tmp);
                        break;
                    case CT_VARCHAR:
                        tmp = tb->select(rid, col);
                        if (strcmp(tmp, tb->buf + tb->head.columnOffset[col]) == 0) {
                            ++conflictCount;
                        }
                        free(tmp);
                        break;
                    default:
                        (false);
                }
            }
            if (conflictCount == tb->head.primaryCount - 1) {
                return false;
            }
            rid = tb->colIndex[firstPrimary].nextEqual(equalFirstIndex);
        }
        return true;
    }
};
#endif