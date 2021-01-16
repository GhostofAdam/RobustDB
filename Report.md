# <数据库系统概论>作业报告

> 桂尚彤 计74
>
> 蒋王一 计71 2017013560

### 编译与运行

#### 编译

```shell
mkdir build; cd build
cmake ..
make
```

#### 运行

```shell
./build/build/RobustDB [dbName] [noCheck] < test.sql;
```

+ `dbName`: 可选参数，指定运行的Database名称；
+ `noCheck`: 可选参数，若选定，则不检查外键、主键等约束条件；

### 系统架构设计

### 模块详细设计

#### 记录管理模块

+ 维护若干个存储数据的文件，实现数据库的可持久化，支持以下操作：
  + 创建文件、删除文件、打开文件、关闭文件；
  + 插入记录、删除记录、更新记录、获取属性值满足特定条件的记录；
  + 将某个文件中的某一页读取到内存、将某个文件中的某一页内容进行更新；
+ `FileManager`类实现页式管理和与文件系统的读写交互；
+ `BufManager`类实现一个`LRU`缓存算法，通过一个缓存队列和写回策略，减少文件IO的次数，提高效率；

#### 索引模块

为文件中的记录建立`B+`树索引以提高查询速度；

+ `B+`树实现：调用开源仓库[stx-btree](https://github.com/bingmann/stx-btree)中实现的`B+`树模板
+ `Index`类：实现索引到`B+`树的映射
  + 增删改索引：修改对应`B+`树
  + 插入/删除列的对应值：在`B+`树中插入/删除节点
  + 值的比较：节点`operator < / >`的重定义

#### 系统管理模块

联合询解析模块实现解析器来解析用户的命令并执行一系列相关的操作。

+ `DBMS`类

  作为`Parser`模块的下游和`Database`模块的上游，起到管理数据库实例，连接`SQL`指令和数据库实例的具体操作的桥梁作用。

  + `current`指针指向当前活跃的数据库实例；
  + 一部分函数，实现对数据库的管理，创建、切换、删除数据库实例；
  + 一部分函数，用以调用当前数据库实例的具体操作，作为`parser-->file`操作流程中的一环；

+ `Database`类

  数据库实例，负责维护具体表格的创建、修改、删除等操作；

+ `Table`类

  核心模块，实现数据库最底层的操作；

  + `TableHead`：存储表格的列、主键、外键、索引等核心信息，维护在表格对应文件的第一页；
  + 部分函数：实现列的增删改；
  + 部分函数：实现记录的增删查改；
  + 部分函数：实现列约束的增删改；
  + 以上操作需配合`ForeignKey`、`PrimaryKey`、`Record`等相关类共同实现；

#### 查询解析模块

参考[开源项目](https://raw.githubusercontent.com/thinkpad20/sql/master/src/lex/sql.l)，基于词法分析器`flex`和语法分析器`bison`实现一个SQL命令的解析器。

+ 在文件`sql.l`中，基于`flex`语法，实现关键词解析(大小写的统一化)和特定变量种类的匹配规则；
+ 在文件`sql.y`中，基于`bison`语法，构建预先规定好的parser文法规则，并根据解析出的不同指令调用预先实现的数据库相关操作函数；
+ 在`Execute`中，为`sql.y`提供相应的数据库操作上层接口，并在相应函数中调用下游`DBMS`模块，完成规定的数据库指令操作；

### 主要接口说明

执行某个`SQL`语句，整个项目的流程如下：

```
SQL Parser --> Execute --> DBMS --> Database --> Table
```

设计语句执行的顶层操作在`Execute`中，底层操作在`Table`中，中间类的函数更多地涉及曾层函数调用，因此我们介绍`Execute`和`Table`中的几个主要接口：

+ `Table::addColumn(const char *name, ColumnType type, bool notNull, bool hasDefault, expr_node *data)`

  功能：为表格添加一列属性

  参数：

  + name：列名称
  + type：列对应的属性
  + notNull：该列是否要求非空
  + hasDefault：该列是否要求有默认值
  + data：若需要有默认值，存储相应的默认值

  操作步骤：

  + 检查表格中是否已出现相同名字的列，如果有，插入失败；
  + 更新`head`中有关列名字、id等信息；
  + 若要求非空，将相应位置1；
  + 若有默认值要求，需将对应的默认值存储在head中开辟的dataArr区域中。我们需要根据该列的具体属性，开辟不同大小的空间，以合理利用head中的有限空间，同时需要存储该默认值在dataArr中的位置，以便需要时调用；

+ `DBMS::insertRow(const char *table, const linked_list *columns, const linked_list *values)`

  功能：向指定表格中添加一行记录

  参数：

  + table：表格名称
  + columns：欲插入的相应列的链表
  + values：与插入的相应数据

  操作步骤：

  + 检查表格和欲添加列名称的合法性；
  + 检查欲添加列与相应数据之间的数量、数据类型合法性；
  + 检查外键约束与主键互斥性；
  + 若以上均合法，调用`Table`类相应的插入函数进行具体操作；

### 实验结果

#### 基本功能

除`SHOW DATABASES`外，完成了parser文法规则中的所有规定文法。

在测试和检查中，出现以下几个问题：

+ DATE数据格式没有进行合法性检查；
+ 查询功能在查询DATE时出现错误，查询其余类型正常；
+ 未支持表的连接，即`<expr> := <col>`

其余功能均能正常执行无误。

以下是一些运行实例展示：

![image-20210116230915133](C:\Users\jiang\AppData\Roaming\Typora\typora-user-images\image-20210116230915133.png)

![image-20210116230850517](C:\Users\jiang\AppData\Roaming\Typora\typora-user-images\image-20210116230850517.png)

![image-20210116230938743](C:\Users\jiang\AppData\Roaming\Typora\typora-user-images\image-20210116230938743.png)

![image-20210116230954962](C:\Users\jiang\AppData\Roaming\Typora\typora-user-images\image-20210116230954962.png)



#### 附加功能

未实现。

### 小组分工

#### 桂尚彤

+ 查询解析模块的实现；
+ 系统管理模块中的部分重要操作，如`search`和外键约束等；

#### 蒋王一

+ 系统管理模块中的部分操作，如增、删、改和主键约束等；
+ 索引模块的实现

### 参考资料

[stx-btree](https://github.com/bingmann/stx-btree)

[sql parser](https://raw.githubusercontent.com/thinkpad20/sql/master/src/lex/sql.l)

整体布局架构参考[SimpleDB](https://github.com/Harry-Chen/SimpleDB/)