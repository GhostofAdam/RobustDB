#include "Database.hpp"

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

string Database::getDBName() {
    return this->dbName;
}

void Database::open(const string &name) {
    this->dbName = name;
    ifstream fin((name + ".db").c_str());
    int tableSize;
    fin >> tableSize;
    for (size_t i = 0; i < tableSize; i++) {
        string _name;
        fin >> _name;
        this->tableName.push_back(_name);
        this->table.push_back(new Table());
        this->table[i]->open((name + "." + this->tableName[i] + ".table").c_str());
    }
    this->ready = true;
}

void Database::close() {
    FILE *file = fopen((this->dbName + ".db").c_str(), "w");
    int tableSize = this->table.size();
    fprintf(file, "%zu\n", tableSize);
    
    for (size_t i = 0; i < tableSize; i++) {
        this->table[i]->close();
        delete this->table[i];
        fprintf(file, "%s\n", this->tableName[i].c_str());
    }
    
    this->table.clear();
    fclose(file);
    this->ready = false;
    
}

void Database::drop() {
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
    fclose(file);
    this->ready = true;
    this->dbName = name;
}

Table *Database::createTable(const std::string &name) {
    tableName.push_back(name);
    table.push_back(new Table());
    table.back()->create((dbName + "." + name + ".table").c_str());
    return table.back();
}

void Database::dropTableByName(const std::string &name) {
    int p = -1;
    for (size_t i = 0; i < table.size(); i++)
        if (!tableName[i].compare(name)) {
            p = (int) i;
            break;
        }
    if (p == -1) {
        printf("[ERROR]Drop Table Error: Table not found!\n");
        return;
    }
    table[p]->drop();
    delete table[p];
    table[p] = table.back();
    table.back() = 0;
    tableName[p] = tableName.back();
    tableName.back() = "";
    remove((dbName + "." + name + ".table").c_str());
}

int Database::getTableId(const string &name) {
    for (size_t i = 0; i < table.size(); i++)
        if (!tableName[i].compare(name)) {
            return i;
        }
}

Table *Database::getTableById(const size_t id) {
    if (id < table.size()) return table[id];
    else return nullptr;
}

Table *Database::getTableByName(const string & name) {
    for (size_t i = 0; i < table.size(); i++){
        if (!tableName[i].compare(name)) {
            return table[i];
        }
    }
    return nullptr;
}

vector<string> Database::getTableNames() {
    return tableName;
}

bool Database::setPrimaryKey(Table* tab, const char* column_name){
    bool succeed = true;
    int t = tab->getColumnID(column_name);
    if (t == -1) {
        printf("[ERROR]Add Primary Key Error: Column %s does not exist\n", column_name);
        return false;
    }
    TableIndex::createIndex(tab, t);
    PrimaryKey::setPrimary(tab, t);
    return true;
}

bool Database::setForeignKey(Table* tab, const char* column_name, const char* foreign_table_name, const char* foreign_column_name){
    int t = tab->getColumnID(column_name);
    if (t == -1) {
        printf("[ERROR]Add Foreign Key Error: Column %s does not exist\n", column_name);
        return false;
    }
    if (tab->getColumnType(t) != CT_INT) {
        printf("[ERROR]Add Foreign Key Error: Column %s must be int.\n", column_name);
        return false;
    }
    auto foreign_table = getTableByName(foreign_table_name);
    if (foreign_table == nullptr) {
        printf("[ERROR]Add Foreign Key Error: Foreign table %s does not exist\n", foreign_table_name);
        return false;
    }
    auto foreign_col = foreign_table->getColumnID(foreign_column_name);
    if (foreign_col == -1) {
        printf("[ERROR]Add Foreign Key Error: Foreign column %s does not exist\n", foreign_column_name);
        return false;
    }
    if (tab->getColumnType(t) != foreign_table->getColumnType(foreign_col)) {
        printf("[ERROR]Add Foreign Key Error: Type of foreign column %s does not match %s\n",
                foreign_column_name, column_name);
        return false;
    }
    if (!foreign_table->hasIndex(foreign_col)) {
        printf("[ERROR]Add Foreign Key Error: Foreign column %s must be indexed.\n", foreign_column_name);
        return false;
    }
    printf("--Add Foreign Key Error: Column %s Foreign Table %s Foreign Column Name %s \n",column_name, foreign_table_name,foreign_column_name);
    ForeignerKey::addForeignKeyConstraint(tab, t, (int) foreign_table->permID, foreign_col);
    return true;
}
void Database::renameTable(const char *old_table, const char *new_table){
    for(int i=0;i<tableName.size();i++){
        if(strcmp(tableName[i].c_str(),old_table)==0){
            printf("--Renamed Table %s to %s\n",old_table,new_table);
            close();
            std::string new_t(new_table);
            rename((this->dbName + "." + this->tableName[i] + ".table").c_str(), (this->dbName + "." + new_t + ".table").c_str());
            open(this->dbName);
            tableName[i] = new_table;
        }
    }
}