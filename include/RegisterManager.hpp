#include "common.hpp"

class Table;

class RegisterManager {
private:
    map<int, Table*> list;
    RegisterManager() = default;
    ~RegisterManager() = default;

public:
    RegisterManager(RegisterManager const &) = delete;
    RegisterManager &operator=(RegisterManager const &) = delete;
    static RegisterManager &getInstance() {
        static RegisterManager instance;
        return instance;
    }
    void checkIn(int permID, Table *table) {
        assert(list.find(permID) == list.end());
        list[permID] = table;
    }
    void checkOut(int permID) {
        assert(list.count(permID) > 0);
        list.erase(permID);
    }
    Table *getPtr(int permID) {
        assert(list.find(permID) == list.end());
        return list[permID];
    }
}; 