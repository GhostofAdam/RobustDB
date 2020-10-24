#include "common.hpp"
#include "Table.hpp"

class Database{
public:
    Database();
    void open();
    void close();
    void drop();
    void create();
    
private:
    vector<Table*> tables_;
};