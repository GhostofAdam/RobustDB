#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>
#include <map>
#include <cstring>
#include <iomanip>
#include <sstream>
#include "type_def.hpp"
#include "DBMS.hpp"

DBMS::DBMS() {
    current = new Database();
}

void DBMS::printReadableException(int err) {
    printf("Exception: ");
    printf("%s\n", Exception2String[err]);
}

void DBMS::printExprVal(const Expression &val) {
    switch (val.type) {
        case TERM_INT:
            printf("%d", val.value.value_i);
            break;
        case TERM_BOOL:
            printf("%s", val.value.value_b ? "TRUE" : "FALSE");
            break;
        case TERM_FLOAT:
            printf("%.2f", val.value.value_f);
            break;
        case TERM_STRING:
            printf("'%s'", val.value.value_s);
            break;
        case TERM_DATE: {
            auto time = (time_t) val.value.value_i;
            auto tm = std::localtime(&time);
            std::cout << std::put_time(tm, DATE_FORMAT);
            break;
        }
        case TERM_NULL:
            printf("NULL");
            break;
        default:
            break;
    }
}

bool DBMS::convertToBool(const Expression &val) {
    bool t = false;
    switch (val.type) {
        case TERM_INT:
            t = val.value.value_i != 0;
            break;
        case TERM_BOOL:
            t = val.value.value_b;
            break;
        case TERM_FLOAT:
            t = val.value.value_f != 0;
            break;
        case TERM_STRING:
            t = strlen(val.value.value_s) != 0;
            break;
        case TERM_NULL:
            t = false;
            break;
        default:
            break;
    }
    return t;
}

Expression DBMS::dbTypeToExprType(char *data, ColumnType type) {
    Expression v;
    if (data == nullptr) {
        return Expression((term_type)TERM_NULL);
    }
    switch (type) {
        case CT_INT:
            v.type = (term_type)TERM_INT;
            v.value.value_i = *(int *) data;
            break;
        case CT_VARCHAR:
            v.type = (term_type)TERM_STRING;
            v.value.value_s = data;
            break;
        case CT_FLOAT:
            v.type = (term_type)TERM_FLOAT;
            v.value.value_f = *(float *) data;
            break;
        case CT_DATE:
            v.type = (term_type)TERM_DATE;
            v.value.value_i = *(int *) data;
            break;
        default:
            printf("Error: Unhandled type\n");
            assert(0);
    }
    return v;
}

term_type DBMS::ColumnTypeToExprType(const ColumnType &type) {
    switch (type) {
        case CT_INT:
            return (term_type)TERM_INT;
        case CT_FLOAT:
            return (term_type)TERM_FLOAT;
        case CT_VARCHAR:
            return (term_type)TERM_STRING;
        case CT_DATE:
            return (term_type)TERM_DATE;
        default:
            throw (int) EXCEPTION_WRONG_DATA_TYPE;
    }
}

bool DBMS::checkColumnType(ColumnType type, const Expression &val) {
    if (val.type == TERM_NULL)
        return true;
    switch (val.type) {
        case TERM_INT:
            return type == CT_INT || type == CT_FLOAT;
        case TERM_FLOAT:
            return type == CT_FLOAT;
        case TERM_STRING:
            return type == CT_VARCHAR;
        case TERM_DATE:
            return type == CT_DATE;
        default:
            return false;
    }
}

char *DBMS::ExprTypeToDbType(Expression &val, term_type desiredType) {
    char *ret = nullptr;
    //TODO: data type convert here, e.g. double->int
    switch (val.type) {
        case TERM_INT:
            if (desiredType == TERM_FLOAT) {
                val.value.value_f = val.value.value_i;
                ret = (char *) &val.value.value_f;
            } else {
                ret = (char *) &val.value.value_i;
            }
            break;
        case TERM_BOOL:
            ret = (char *) &val.value.value_b;
            break;
        case TERM_FLOAT:
            if (desiredType == TERM_INT) {
                val.value.value_i = (int) val.value.value_f;
                ret = (char *) &val.value.value_i;
            } else {
                ret = (char *) &val.value.value_f;
            }
            break;
        case TERM_STRING:
            ret = val.value.value_s;
            break;
        case TERM_DATE:
            ret = (char *) &val.value.value_i;
            break;
        case TERM_NULL:
            ret = nullptr;
            break;
        default:
            printf("Error: Unhandled type\n");
            assert(false);
    }
    return ret;
}

void DBMS::cacheColumns(Table *tb, int rid) {
    auto tb_name = tb->getTableName();
    tb_name = tb_name.substr(tb_name.find('.') + 1); //strip database name
    tb_name = tb_name.substr(0, tb_name.find('.'));
    cleanColumnCacheByTable(tb_name.c_str());
    for (int i = 1; i <= tb->getColumnCount() - 1; ++i)//exclude RID
    {
        auto *tmp = tb->select(rid, i);
        updateColumnCache(tb->getColumnName(i),
                          tb_name.c_str(),
                          dbTypeToExprType(tmp, tb->getColumnType(i))
        );
        pendingFree.push_back(tmp);
    }
}

void DBMS::freeCachedColumns() {
    for (const auto &ptr : pendingFree) {
        delete (ptr);
    }
    pendingFree.clear();
}

DBMS::IDX_TYPE DBMS::checkIndexAvailability(Table *tb, RID_t *rid_l, RID_t *rid_u, int *col, expr_node *condition) {
    //TODO: complex conditions
    if (condition && condition->node_type == TERM_NONE && condition->op == OPER_AND)
        condition = condition->left;
    if (!(condition && condition->node_type == TERM_NONE && condition->left->node_type == TERM_COLUMN))
        return IDX_NONE;
    auto col_name = condition->left->column->column;
    int c = tb->getColumnID(col_name);
    /*if(c == -1){
        c = tb->getColumnID(condition->right->column->column);
    }*/
    if (c == -1 || !tb->hasIndex(c))
        return IDX_NONE;
    Expression v;
    try {
        v = calcExpression(condition->right);
    } catch (int err) {
        printReadableException(err);
        return IDX_NONE;
    }
    IDX_TYPE type;
    switch (condition->op) {
        case OPER_EQU:
            type = IDX_EQUAL;
            break;
        case OPER_LT:
        case OPER_LE:
            type = IDX_UPPER;
            break;
        case OPER_GT:
        case OPER_GE:
            type = IDX_LOWWER;
            break;
        default:
            type = IDX_NONE;
    }
    if (type != IDX_NONE) {
        *col = c;
        auto colType = ColumnTypeToExprType(tb->getColumnType(c));
        *rid_u = tb->selectIndexUpperBound(c, ExprTypeToDbType(v, colType));
        *rid_l = tb->selectIndexLowerBound(c, ExprTypeToDbType(v, colType));
    }
    return type;
}

RID_t DBMS::nextWithIndex(Table *tb, IDX_TYPE type, int col, RID_t rid, RID_t rid_u) {
    if (type == IDX_EQUAL) {
        auto nxt = tb->selectIndexNext(col);
        return rid == rid_u ? (RID_t) -1 : nxt; // current rid equals upper bound
    } else if (type == IDX_UPPER)
        return tb->selectReveredIndexNext(col);
    else if (type == IDX_LOWWER)
        return tb->selectIndexNext(col);
    else {
        return tb->getNext(rid);
    }
}


expr_node *DBMS::findJoinCondition(expr_node *condition) {
    expr_node *cond = nullptr;
    if (condition->left->node_type == TERM_COLUMN && condition->right->node_type == TERM_COLUMN) {
        cond = condition;
    } else if (condition->left->node_type == TERM_NONE) {
        cond = findJoinCondition(condition->left);
    } else if (!cond && condition->right->node_type == TERM_NONE) {
        cond = findJoinCondition(condition->right);
    }
    return cond;
}




void DBMS::freeLinkedList(linked_list *t) {
    linked_list *next;
    for (; t; t = next) {
        next = t->next;
        free(t);
    }
}

DBMS *DBMS::getInstance() {
    static DBMS *instance;
    if (!instance)
        instance = new DBMS;
    return instance;
}

void DBMS::exit() {
    if (current->isOpen())
        current->close();
}

void DBMS::switchToDB(const char *name) {
    if (current->isOpen())
        current->close();

    current->open(name);
}

void DBMS::createTable(const table_def *table) {
    if (!current->isOpen()){
        printf("Current Database is not open!\n");
        return;
    }
        
    assert(table->name != NULL);
    const char * name = (const char *) table->name;
    if (current->getTableByName(name)) {
        printf("Table `%s` already exists\n", table->name);
        return;
    }
    Table *tab = current->createTable(table->name);
    std::vector<column_defs *> column_vec;
    column_defs *column = table->columns;
    bool succeed = true;
    while (column){
        column_vec.push_back(column);
        column = column->next;
    }
    std::reverse(column_vec.begin(),column_vec.end());
    for (auto i = column_vec.begin(); i != column_vec.end(); ++i) {
        auto type = (ColumnType) 0;
        switch ((*i)->type) {
            case COLUMN_TYPE_INT:
                type = CT_INT;
                break;
            case COLUMN_TYPE_VARCHAR:
                type = CT_VARCHAR;
                break;
            case COLUMN_TYPE_FLOAT:
                type = CT_FLOAT;
                break;
            case COLUMN_TYPE_DATE:
                type = CT_DATE;
                break;
            default:
                assert(0==1);
                break;
        }
        
        column->flags->default_value;
        int id = tab->addColumn(column->name, type,
                                 (bool) column->flags->flags & COLUMN_FLAG_NOTNULL,
                                 (bool) column->flags->flags & COLUMN_FLAG_DEFAULT,
                                 column->flags->default_value);
        succeed = id == -1;
        if(!succeed){
            printf("Column %s exits\n", column->name);
            break;
        }
    }

    auto *cons_list = table->constraints;
    while (cons_list){
        table_constraint *cons = (table_constraint *) (cons_list->data);
        switch (cons->type) {
            case CONSTRAINT_PRIMARY_KEY: {
                auto *column_name = cons->column_list;
                while (column_name){
                    auto name = ((column_ref *) column_name->data)->column;
                    printf("Adding primary key constraint: Column: %s\n", name);
                    succeed = current->setPrimaryKey(tab, name);
                    column_name = column_name->next;
                }
                break;
            }
            case CONSTRAINT_FOREIGN_KEY: {
                // printf("Foreign key: COLUMN %s REFERENCES TABLE %s COLUMN %s\n",
                //        cons->column_name, cons->foreign_table_name, cons->foreign_column_name);
                // succeed = current->setForeignKey(tab, cons->column_list, cons->foreign_table_name, cons->foreign_column_list);
                break;
            }
            default:
                assert(0); // WTF?
        }
        cons_list = cons_list->next;
    }

    if (!succeed)
        current->dropTableByName(table->name);
    else
        printf("Table %s created\n", table->name);
}

void DBMS::dropDB(const char *db_name) {
    Database db;
    if (current->isOpen() && current->getDBName() == db_name)
        current->close();
    db.open(db_name);
    if (db.isOpen()) {
        db.drop();
        printf("Database %s dropped!\n", db_name);
    } else {
        printf("Failed to open database %s\n", db_name);
    }
}

void DBMS::dropTable(const char *table) {
    if (!current->isOpen()){
        printf("Database is not open!");
        return;
    }
    current->dropTableByName(table);
    printf("Table %s dropped!\n", table);
}

void DBMS::showTables() {
    if (!current->isOpen()){
        printf("Database is not open!");
        return;
    }
    printf("Show all the tables:\n");
    printf("***********\n");
    for (auto table : current->getTableNames()) {
        printf("%s\n", table.c_str());
    }
    printf("***********\n");
}

void DBMS::selectRow(const linked_list *tables, const linked_list *column_expr, expr_node *condition) {
    int flags;
    if (!current->isOpen()){
        printf("Database is not open!");
        return;
    }
    linked_list *openedTables = nullptr;
    bool allOpened = true;
    for (; tables; tables = tables->next) {
        Table *tb;
        auto *table = (const char *) tables->data;
        if (!(tb = current->getTableByName(table))) {
            printf("Table %s not found\n", table);
            allOpened = false;
        }
        auto *t = (linked_list *) malloc(sizeof(linked_list));
        t->next = openedTables;
        t->data = tb;
        openedTables = t;
    }
    if (!allOpened) {
        freeLinkedList(openedTables);
        return;
    }
    auto results =  selectRidfromTables(openedTables, condition);
    for(auto row : results){
        // TODO
    }
    freeLinkedList(openedTables);
}

void DBMS::updateRow(const char *table, expr_node *condition, column_ref *column, expr_node *eval) {
    Table *tb;
    if (!current->isOpen()){
        printf("Database is not open!");
        return;
    }
    if (!(tb = current->getTableByName(table))) {
        printf("Table %s not found\n", table);
        return;
    }

    int col_to_update;
    col_to_update = tb->getColumnID(column->column);
    if (col_to_update == -1) {
        printf("Column %s not found\n", column->column);
        return;
    }
    auto resutls = selectRidfromTable(tb, condition);
    for(auto i: resutls){
        //TODO
    }
    
    printf("%d rows updated.\n", (int )resutls.size());
}

void DBMS::deleteRow(const char *table, expr_node *condition) {
    std::vector<RID_t> toBeDeleted;
    Table *tb;
    if (!current->isOpen()){
        printf("Database is not open!");
        return;
    }
    if (!(tb = current->getTableByName(table))) {
        printf("Table %s not found\n", table);
        return;
    }
    toBeDeleted = selectRidfromTable(tb, condition);
    for (const auto &i : toBeDeleted) {
        tb->dropRecord(i);
    }
    printf("%d rows deleted.\n", (int) toBeDeleted.size());
}

void DBMS::insertRow(const char *table, const linked_list *columns, const linked_list *values) {
    
    if (!current->isOpen()){
        printf("Database is not open!");
        return;
    }
    Table *tb =  current->getTableByName(table);
    if (!tb) {
        printf("Table %s not found\n", table);
        return;
    }
    std::vector<int> colId;
    if (!columns) { // 没有特别指定列
        for (int i = tb->getColumnCount() - 1; i > 0; --i) { //exclude RID
            colId.push_back(i);
        }
    } else{        
        for (const linked_list *i = columns; i; i = i->next) {
            const column_ref *col = (column_ref *) i->data;
            auto name = col->column;
            int id = tb->getColumnID(name);
            if (id < 0) {
                printf("Column %s not found\n", col->column);
                return;
            }
            colId.push_back(id);
        }
    }
    printf("Inserting into %lu columns\n", colId.size());
    tb->clearBuffer();
    int count = 0;
    for (const linked_list *i = values; i; i = i->next) {
        const linked_list *expr_list = (linked_list *) i->data;
        unsigned int cnt = 0;
        for (const linked_list *j = expr_list; j; j = j->next) {
            cnt++;
        }
        if (cnt != colId.size()) {
            printf("Column size mismatch, will not execute (value size=%d)\n", cnt);
            continue;
        }
        auto it = colId.begin();
        bool succeed = false;
        for (const linked_list *j = expr_list; j; j = j->next) {
            auto node = (expr_node *) j->data;
            Expression val;
            try {
                val = calcExpression(node);
            } catch (int err) {
                printReadableException(err);
                return;
            } catch (...) {
                printf("Exception occur %d\n", __LINE__);
                return;
            }
            auto colType = tb->getColumnType(*it);
            if (!checkColumnType(colType, val)) {
                printf("Wrong data type\n");
                return;
            }
            auto exprType = ColumnTypeToExprType(colType);
            succeed = tb->insert2Buffer(*it, ExprTypeToDbType(val, exprType));
            if (!succeed) {
                break;
            }
            ++it;
        }
        if(!succeed){
            break;
        }
        else{
            ++count;
        }
        succeed = tb->insert2Record();
    }
    printf("%d rows inserted.\n", count);
}

void DBMS::addColumn(const char *table, struct column_defs *col_def) {
    Table *tb;
    if (!requireDbOpen())
        return;
    if (!(tb = current->getTableByName(table))) {
        printf("Table %s not found\n", table);
        return;
    }
    while (col_def != nullptr) {
        int id = tb->addColumn(col_def->name,
                              (ColumnType)col_def->type,
                              (bool) col_def->flags & COLUMN_FLAG_NOTNULL,
                              (bool) col_def->flags & COLUMN_FLAG_DEFAULT,
                              nullptr);
        col_def = col_def->next;
        if (id == -1) {
            printf("Column %s exits\n", col_def->name);
        }
    }
}

void DBMS::dropColumn(const char *table, struct column_ref *tb_col) {
    Table *tb;
    if (!requireDbOpen())
        return;
    if (!(tb = current->getTableByName(table))) {
        printf("Table %s not found\n", table);
        return;
    }
    int id = tb->dropColumn(tb_col->column);
    if (id == -1) {
        printf("Column %s doesn't exist\n", tb_col->column);
    }
}

void DBMS::renameColumn(const char *table, const char *old_col, const char *new_col) {
    Table *tb;
    if (!requireDbOpen())
        return;
    if (!(tb = current->getTableByName(table))) {
        printf("Table %s not found\n", table);
        return;
    }
    int id = tb->renameColumn(old_col, new_col);
    if (id == -1) {
        printf("Column %s doesn't exist\n", old_col);
    }
}

void DBMS::addPrimary(const char *table, const char *col) {
    Table *tb;
    if (!requireDbOpen())
        return;
    if (!(tb = current->getTableByName(table))) {
        printf("Table %s not found\n", table);
        return;
    }
    int id = tb->addPrimary(col);
    if (id == -1) {
        printf("Column %s doesn't exist\n", col);
    }
}

void DBMS::dropPrimary(const char *table) {
    Table *tb;
    if (!requireDbOpen())
        return;
    if (!(tb = current->getTableByName(table))) {
        printf("Table %s not found\n", table);
        return;
    }
    tb->dropPrimary();
}

void DBMS::dropPrimary_byname(const char *table, const char *col) {
    Table *tb;
    if (!requireDbOpen())
        return;
    if (!(tb = current->getTableByName(table))) {
        printf("Table %s not found\n", table);
        return;
    }
    int id = tb->dropPrimary_byname(col);
    if (id == -1) {
        printf("Column %s doesn't exist\n", col);
    }
}

void DBMS::addConstraint(const char *table, const char *cons_name, table_constraint *cons) {
    Table *tb, *tb_cons;
    if (!requireDbOpen())
        return;
    if (!(tb = current->getTableByName(table))) {
        printf("Table %s not found\n", table);
        return;
    }
    if (!(tb_cons = current->getTableByName(cons->foreign_table_name))) {
        printf("Foreign Table %s not found\n", cons->foreign_table_name);
        return;
    }
    int col_id = tb->getColumnID(cons->column_name);
    int foreign_table_id = current->getTableId(cons->foreign_table_name);
    int foreign_col_id = tb_cons->getColumnID(cons->foreign_column_name);
    tb->addForeignKeyConstraint(col_id, foreign_table_id, foreign_col_id);
}

void DBMS::dropForeign(const char *table) {
    Table *tb;
    if (!requireDbOpen())
        return;
    if (!(tb = current->getTableByName(table))) {
        printf("Table %s not found\n", table);
        return;
    }
    tb->dropForeign();
}

void DBMS::createIndex(column_ref *tb_col) {
    Table *tb;
    if (!current->isOpen()){
        printf("Database is not open!");
        return;
    }
    auto name = (const char*) tb_col->table;
    if (!(tb = current->getTableByName(tb_col->table))) {
        printf("Table %s not found\n", tb_col->table);
        return;
    }
    int t = tb->getColumnID(tb_col->column);
    if (t == -1) {
        printf("Column %s not exist\n", tb_col->column);
    } else
        tb->createIndex(t);
}

void DBMS::dropIndex(column_ref *tb_col) {
    Table *tb;
    if (!current->isOpen()){
        printf("Database is not open!");
        return;
    }
    if (!(tb = current->getTableByName(tb_col->table))) {
        printf("Table %s not found\n", tb_col->table);
        return;
    }
    int t = tb->getColumnID(tb_col->column);
    if (t == -1) {
        printf("Column %s not exist\n", tb_col->column);
    } else if (!tb->hasIndex(t)) {
        printf("No index on %s(%s)\n", tb_col->table, tb_col->column);
    } else {
        tb->dropIndex(t);
    }
}

void DBMS::descTable(const char *name) {
    Table *tb;
    if (!current->isOpen()){
        printf("Database is not open!\n");
        return;
    }
    Table *tb = current->getTableByName(name);
    if (!tb) {
        printf("Table name %s not found\n", name);
        return;
    }
    tb->printTableDef();
}

bool DBMS::valueExistInTable(const char *value, const ForeignKey &key) {
    auto table = current->getTableById(key.foreign_table_id);
    auto result = table->selectIndexLowerBoundEqual(key.foreign_col, value);
    return result != (RID_t) -1;
}