#ifndef __FK_HPP__
#define __FK_HPP__
#include "Table.hpp"

class ForeignerKey{
public:
    static int dropForeignByName(Table* tb, const char *fk_name){
        int8_t fkt = tb->head.foreignKeyTot;
        bool fk_flag[fkt];
        for (int8_t i = 0; i < fkt; i++) {
            if (strcmp(tb->head.foreignKeyList[i].name, fk_name) == 0)
                fk_flag[i] = false;
            else
                fk_flag[i] = true;
        }
        tb->head.foreignKeyTot = 0;
        for (int8_t i = 0; i < fkt; i++) {
            if (fk_flag[i]) {
                tb->head.foreignKeyList[tb->head.foreignKeyTot] = tb->head.foreignKeyList[i];
                tb->head.foreignKeyTot++;
            }
        }
    }
    static void addForeignKeyConstraint(Table* tb, unsigned int col, unsigned int foreignTableId, 
    unsigned int foreignColId, const char* fk_name = nullptr){
        tb->head.foreignKeyList[tb->head.foreignKeyTot].col = col;
        tb->head.foreignKeyList[tb->head.foreignKeyTot].foreign_table_id = foreignTableId;
        tb->head.foreignKeyList[tb->head.foreignKeyTot].foreign_col = foreignColId;
        if(fk_name != nullptr)
            strcpy(tb->head.foreignKeyList[tb->head.foreignKeyTot].name, fk_name);
        tb->head.foreignKeyTot++;
    }
    static void dropForeign(Table* tb){
        printf("--Drop ALL Foreign Key Total %d\n", tb->head.foreignKeyTot);
        tb->head.foreignKeyTot = 0;
    }
};
#endif