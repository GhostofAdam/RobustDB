#include "common.hpp"

typedef struct linked_list {
    void *data;
    struct linked_list *next;
} linked_list;

typedef struct column_defs {
    char *name;
    int type;
    int size;
    unsigned int flags;
    struct column_defs *next;
} column_defs;

typedef struct table_def {
    char *name;
    column_defs *columns;
    linked_list *constraints;
} table_def;