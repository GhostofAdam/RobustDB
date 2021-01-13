#include <cstdio>
#include <cstdlib>
#include "Execute.h"

#include "../backend/DBMS.hpp"

void free_expr(expr_node* node){
    
}

void free_condition_tree(condition_tree* node){
    
}

void free_column_ref(column_ref *c) {
    if (c->table)
        free(c->table);
    free(c->column);
    free(c);
}

void free_column_list(linked_list *cols) {
    while (cols) {
        auto c = (column_ref *) cols->data;
        free_column_ref(c);
        linked_list *t = cols;
        cols = cols->next;
        free(t);
    }
}

void free_expr_list(linked_list *exprs) {
    while (exprs) {
        auto e = (expr_node *) exprs->data;
        free_expr(e);
        linked_list *t = exprs;
        exprs = exprs->next;
        free(t);
    }

}

void free_values(linked_list *values) {
    while (values) {
        auto exprs = (linked_list *) values->data;
        free_expr_list(exprs);
        linked_list *t = values;
        values = values->next;
        free(t);
    }
}

void free_tables(linked_list *tables) {
    while (tables) {
        auto table_name = (char *) tables->data;
        free(table_name);
        linked_list *t = tables;
        tables = tables->next;
        free(t);
    }
}

void report_sql_error(const char *error_name, const char *msg) {
    printf("SQL Error[%s]: %s\n", error_name, msg);
}

void execute_desc_tables(const char *table_name) {
    printf("desc table %s\n", table_name);
    DBMS::getInstance()->descTable(table_name);
    free((void *) table_name);
}

void execute_show_tables() {
    printf("show tables\n");
    DBMS::getInstance()->showTables();
}

void execute_create_db(const char *db_name) {
    printf("create db %s\n",db_name);
    Database db;
    db.create(db_name);
    db.close();
    free((void *) db_name);
}

void execute_create_tb(const table_def *table) {
    printf("create tb %s\n", table->name);
    DBMS::getInstance()->createTable(table);
    free((void *) table->name);
    column_defs *c = table->columns;
    while (c) {
        column_defs *next = c->next;
        free((void *) c->name);
        free((void *) c->flags->default_value);
        free((void *) c->flags);
        free((void *) c);
        c = next;
    }
    linked_list *cons = table->constraints;
    while (cons) {
        linked_list *next = cons->next;
        auto *tc = (table_constraint *) (cons->data);
        switch(tc->type){
            case CONSTRAINT_FOREIGN_KEY:
                free_column_list(tc->foreign_column_list);
                free(tc->foreign_table_name);
                break;
            case CONSTRAINT_PRIMARY_KEY:
                free_column_list(tc->column_list);
                break;
        }
        free_column_list(tc->column_list);
        free(tc);
        free(cons);
        cons = next;
    }
}

void execute_drop_db(const char *db_name) {
    printf("drop db %s\n", db_name);
    DBMS::getInstance()->dropDB(db_name);
    free((void *) db_name);
}

void execute_drop_table(const char *table_name) {
    printf("drop tb %s\n", table_name);
    DBMS::getInstance()->dropTable(table_name);
    free((void *) table_name);
}

void execute_use_db(const char *db_name) {
    printf("use db %s\n", db_name);
    DBMS::getInstance()->switchToDB(db_name);
    free((void *) db_name);
}

void execute_insert_row(struct insert_argu *stmt) {
    printf("insert row\n");
    assert(stmt->table);
    DBMS::getInstance()->insertRow(stmt->table, stmt->columns, stmt->values);
    free_column_list(stmt->columns);
    free_values(stmt->values);
    free((void *) stmt->table);
}

void execute_select(struct select_argu *stmt) {
    printf("select row\n");
    DBMS::getInstance()->selectRow(stmt->tables, stmt->column_expr, stmt->where);
    free_tables(stmt->tables);
    free_expr_list(stmt->column_expr);
    if (stmt->where)
        free(stmt->where);
}

void execute_delete(struct delete_argu *stmt) {
    printf("delete row\n");
    DBMS::getInstance()->deleteRow(stmt->table, stmt->where);
    free(stmt->table);
    if (stmt->where)
        free_condition_tree(stmt->where);
}

void execute_update(struct update_argu *stmt) {
    printf("update row\n");
    DBMS::getInstance()->updateRow(stmt->table, stmt->where, stmt->column, stmt->val_expr);
    free(stmt->table);
    if (stmt->where)
        free_condition_tree(stmt->where);
    free_column_ref(stmt->column);
    free_expr(stmt->val_expr);
}

void execute_drop_idx(struct column_ref *tb_col) {
    DBMS::getInstance()->dropIndex(tb_col);
    free_column_ref(tb_col);
}

void execute_create_idx(struct column_ref *tb_col) {
    DBMS::getInstance()->createIndex(tb_col);
    free_column_ref(tb_col);
}

void execute_sql_eof() {
    DBMS::getInstance()->exit();
}

void execute_add_column(const char *tb_name, struct column_defs *col_def) {
    DBMS::getInstance()->addColumn(tb_name, col_def);
}

void execute_drop_column(const char *tb_name, struct column_ref *tb_col) {
    DBMS::getInstance()->dropColumn(tb_name, tb_col);
}

// void execute_rename_column(const char *tb_name, const char *old_col, const char *new_col) {
//     DBMS::getInstance()->renameColumn(tb_name, old_col, new_col);
// }

void execute_add_primary_key(const char *tb_name, const char *col) {
    DBMS::getInstance()->addPrimary(tb_name, col);
}

void execute_add_constraint(const char *tb_name, const char *cons_name, table_constraint *cons) {
    DBMS::getInstance()->addConstraint(tb_name, cons_name, cons);
}

void execute_drop_primary_key_byname(const char *tb_name, const char *col) {
    DBMS::getInstance()->dropPrimary_byname(tb_name, col);
}

void execute_drop_primary_key(const char *tb_name) {
    DBMS::getInstance()->dropPrimary(tb_name);
}

void execute_drop_foreign_key(const char *tb_name){
    DBMS::getInstance()->dropForeign(tb_name);
}
void execute_rename_table(const char *old_table, const char *new_table){
    

}

void execute_drop_foreign_key_byname(const char *tb_name, const char *key_name){
    
}