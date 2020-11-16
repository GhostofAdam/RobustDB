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