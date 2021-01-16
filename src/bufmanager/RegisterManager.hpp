#include "../utils/common.hpp"

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
        list[permID] = table;
    }
    void checkOut(int permID) {
        list.erase(permID);
    }
    Table *getPtr(int permID) {
        return list[permID];
    }
}; 