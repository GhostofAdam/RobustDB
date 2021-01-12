README

+ `rename_column`是否需要加上表名

  没有rename_column了，只有rename_table

+ `add_colummn`中的参数size

  不用了

+ `drop_primary_key` 是所有主键都去掉吗？

  是的

+ `drop_foreign_key`是所有主键都去掉吗？需要像`drop_primary_key_byname`来一个指定的吗？

  是的

  文法里没有

+ `execute_rename_column`是不是少了一个`tb_name`?

  没有rename_column了，只有rename_table

+ `struct table_constraint`中的`column_list`有啥用，一次为多个col添加外键？

  column_list现在代表主键或者外键本表中对应的列

+ `execute_add_constraint`中的`cons_name`有啥用？

  primary_key或者foreign_key的name