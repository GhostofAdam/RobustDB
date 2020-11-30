#include "DBMS.hpp"

DBMS::DBMS() {
    current = new Database();
}


bool DBMS::requireDbOpen() {
    if (!this->current->isOpen()) {
        printf("%s\n", "Please USE database first!");
        return false;
    }
    return true;
}

void DBMS::exit() {
    if (this->current->isOpen())    this->current->close();
}

void DBMS::switchToDB(const string name) {
    if (this->current->isOpen())    this->current->close();
    this->current->open(name);
}

void DBMS::createTable(const table_def table) {
    if (!requireDbOpen())
        return;
    Table *tab = current->createTable(table.name);
}

void DBMS::dropDB(const string db_name) {
    Database db;
    if (this->current->isOpen() && this->current->getDBName() == db_name)   this->current->close();
    db.open(db_name);
    if (db.isOpen()) {
        db.drop();
        printf("Database %s dropped!\n", db_name);
    } else {
        printf("Failed to open database %s\n", db_name);
    }
}

void DBMS::dropTable(const string table) {
    if (!requireDbOpen())   return;
    this->current->dropTableByName(table);
    printf("Table %s dropped!\n", table);
}

void DBMS::listTables() {
    if (!requireDbOpen())   return;
    const std::vector<std::string> &tables = this->current->getTableNames();
    printf("List of tables:\n");
    for (const auto &table : tables) {
        printf("%s\n", table.c_str());
    }
    printf("==========\n");
}

void DBMS::createIndex(column_ref *tb_col) {
    Table *tb;
    if (!requireDbOpen())
        return;
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
    if (!requireDbOpen())
        return;
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