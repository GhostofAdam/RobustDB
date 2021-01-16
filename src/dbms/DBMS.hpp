#ifndef __DBMS_HPP__
#define __DBMS_HPP__
#include "../database/Database.hpp"

enum IDX_TYPE {
    IDX_NONE, IDX_LOWWER, IDX_UPPER, IDX_EQUAL
};
using table_value_t = std::pair<std::string, expr_node>;

class DBMS{
public:
    static DBMS* getInstance();
    void exit();
    void switchToDB(const char *name);
    void dropDB(const char *db_name);
    void createTable(const table_def *table);
    void dropTable(const char *table);
    void showTables();
    void selectRow(const linked_list *tables, const linked_list *column_expr, condition_tree *condition);
    void updateRow(const char *table, condition_tree *condition, column_ref *column, expr_node *eval);
    void deleteRow(const char *table, condition_tree *condition);
    void insertRow(const char *table, const linked_list *columns, const linked_list *values);
    void addColumn(const char *table, struct column_defs *col_def);
    void dropColumn(const char *table, struct column_ref *tb_col);
    void renameColumn(const char *table, const char *old_col, const char *new_col);
    void renameTable(const char *old_table, const char *new_table);
    void changeColumn(const char *tb_name, const char *col, struct column_defs *col_def);
    void addPrimary(const char *table, const char *col);
    void dropPrimary(const char *table);
    void dropPrimaryByName(const char *table, const char *col);
    void dropForeignByName(const char *table, const char *fk_name);
    bool addConstraint(const char *table, const char *cons_name, table_constraint *cons);
    void dropForeign(const char *table);
    void createIndex(index_argu *tb_col);
    void dropIndex(index_argu *tb_col);
    void descTable(const char *name);
    bool valueExistInTable(const char* value, const ForeignKey& key);
    ListNode* mergeTable(ListNode* head1, ListNode* head2);
    void binaryTableMerge(vector<ListNode*>& lists);
    ListNode* mergeTableLists(vector<ListNode*>& lists);
    ListNode* visitTableLists(vector<ListNode*>& lists);
    void RecordCount(index_argu *idx_stmt, string s, vector<string>& words);

private:
        enum IDX_TYPE {
        IDX_NONE, IDX_LOWWER, IDX_UPPER, IDX_EQUAL
    };
    Database *current;
    DBMS();
    std::vector<char *> pendingFree;
    std::multimap<std::string, table_value_t> column_cache;
    void printReadableException(int err);
    expr_node dbTypeToExprType(char *data, ColumnType type);
    char *ExprTypeToDbType(const expr_node *val, term_type desiredType);
    term_type ColumnTypeToExprType(const ColumnType& type);
    bool checkColumnType(ColumnType type, const expr_node *val); 
    void cacheColumns(Table *tb, int rid);
    void freeCachedColumns();
    IDX_TYPE checkIndexAvailability(Table *tb, RID_t *rid_l, RID_t *rid_u, int *col, condition_tree *condition);
    RID_t nextWithIndex(Table *tb, IDX_TYPE type, int col, RID_t rid, RID_t rid_u);
    void freeLinkedList(linked_list *t);
    std::vector<RID_t> selectRidfromTable(Table* openedTables, condition_tree *condition);
    bool checkCondition(RID_t rid, condition_tree *condition);
    void updateColumnCache(const char *col_name, const char *table, const expr_node &v);
    void cleanColumnCacheByTable(const char *table);
};
#endif