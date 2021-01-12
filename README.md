README

+ `rename_column`是否需要加上表名
+ `add_colummn`中的参数size
+ `drop_primary_key` 是所有主键都去掉吗？
+ `drop_foreign_key`是所有主键都去掉吗？需要像`drop_primary_key_byname`来一个指定的吗？
+ `execute_rename_column`是不是少了一个`tb_name`?
+ `struct table_constraint`中的`column_list`有啥用，一次为多个col添加外键？
+ `execute_add_constraint`中的`cons_name`有啥用？