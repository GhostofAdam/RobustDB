#include "Database.hpp"
#include "parser/Expression.hpp"

enum IDX_TYPE {
    IDX_NONE, IDX_LOWWER, IDX_UPPER, IDX_EQUAL
};
class DBMS{
public:
    DBMS();
    static DBMS* getInstance();
    void exit();
    void switchToDB(const char *name);
    void dropDB(const char *db_name);
    void createTable(const table_def *table);
    void dropTable(const char *table);
    void showTables();
    void selectRow(const linked_list *tables, const linked_list *column_expr, expr_node *condition);
    void updateRow(const char *table, expr_node *condition, column_ref *column, expr_node *eval);
    void deleteRow(const char *table, expr_node *condition);
    void insertRow(const char *table, const linked_list *columns, const linked_list *values);
    void addColumn(const char *table, struct column_defs *col_def);
    void createIndex(column_ref *tb_col);
    void dropIndex(column_ref *tb_col);
    void descTable(const char *name);
    bool valueExistInTable(const char* value, const ForeignKey& key);
private:
        enum IDX_TYPE {
        IDX_NONE, IDX_LOWWER, IDX_UPPER, IDX_EQUAL
    };
    Database *current;
    std::vector<char *> pendingFree;
    DBMS();
    bool requireDbOpen();
    void printReadableException(int err);
    void printExprVal(const Expression &val);
    bool convertToBool(const Expression &val);
    Expression dbTypeToExprType(char *data, ColumnType type);
    char *ExprTypeToDbType(Expression &val, term_type desiredType);
    term_type ColumnTypeToExprType(const ColumnType& type);
    bool checkColumnType(ColumnType type, const Expression &val); 
    void cacheColumns(Table *tb, int rid);
    void freeCachedColumns();
    IDX_TYPE checkIndexAvailability(Table *tb, RID_t *rid_l, RID_t *rid_u, int *col, expr_node *condition);
    RID_t nextWithIndex(Table *tb, IDX_TYPE type, int col, RID_t rid, RID_t rid_u);
    expr_node* findJoinCondition(expr_node *condition);
    using CallbackFunc = std::function<void(Table *, RID_t)>;
    bool iterateTwoTableRecords(Table *a, Table *b, expr_node *condition, CallbackFunc callback);
    void iterateRecords(linked_list *tables, expr_node *condition, CallbackFunc callback);
    void iterateRecords(Table *tb, expr_node *condition, CallbackFunc callback);
    void freeLinkedList(linked_list *t);
};