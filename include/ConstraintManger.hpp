#include "Table.hpp"

class ConstraintManger{
    static void addForeignKeyConstraint(unsigned int col, unsigned int foreignTableId, unsigned int foreignColId);
};