#include <fsream>
#include <vector>

#include "Database.h"

Database::Database() {
    this->ready = false;
    this->tableName.clear();
    this->table.clear();
}

Database::~Database() {
    if (this->ready) close();
}

bool Database::isOpen() {
    return this->ready;
}

void Database::open(const string &name) {
    assert(!this->ready);
    this->dbName = name;
    ifstream fin((name + ".db").c_str());
    for (size_t i = 0; i < this->tables.size(); i++) {
        fin >> this->tableName[i];
        assert(this->table[i] == nullptr);
        this->table[i] = new Table();
        this->table[i]->open((name + "." + this->tableName[i] + ".table").c_str());
    }
    this->ready = true;
}

void Database::close() {
    assert(this->ready);
    FILE *file = fopen((this->dbName + ".db").c_str(), "w");
    fprintf(file, "%zu\n", this->table.size());
    for (size_t i = 0; i < this->table.size(); i++) {
        this->table[i]->close();
        delete this->table[i];
        this->table[i] = nullptr;
        fprintf(file, "%s\n", this->tableName[i].c_str());
    }
    this->table.clear();
    fclose(file);
    this->ready = false;
}

void Database::drop() {
    assert(this->ready);
    remove((this->dbName + ".db").c_str());
    for (size_t i = 0; i < this->table.size(); i++) {
        this->table[i]->drop();
        delete this->table[i];
        this->table[i] = nullptr;
        remove((dbName + "." + tableName[i] + ".table").c_str());
    }
    this->table.clear();
    this->ready = false;
}

void Database::create(const string &name) {
    auto file = fopen((name + ".db").c_str(), "w");
    assert(file);
    fclose(file);
    assert(this->ready == 0);
    this->ready = true;
    this->dbName = name;
}