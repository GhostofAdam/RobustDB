%define parse.error verbose

%{
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Execute.hpp"
#include "type_def.hpp"

int yyerror(const char *str);

#include "sql.yy.c"

%}
%union {
  int val_i;
  float val_f;
  char *val_s;
  column_ref* ref_column;
  column_defs* def_column;
  table_def* def_table;
  linked_list* list;
  insert_argu* insert_argu;
  expr_node* expr_node;
  select_argu* select_argu;
  delete_argu* delete_argu;
  update_argu* update_argu;
  table_field* t_field;
  table_constraint* t_constraint;
}

%token TRUE FALSE AND OR NEQ GEQ LEQ NOT
%token IS INDEX DATABASE DROP USE UPDATE SET
%token TOKEN_NULL INT DOUBLE CHAR VARCHAR USING
%token DESC
%token UNIQUE ON VALUES
%token TABLE
%token CREATE SELECT WHERE INSERT INTO FROM
%token DEFAULT PRIMARY FOREIGN KEY REFERENCES
%token DELETE SHOW
%token IDENTIFIER FLOAT DATE EXIT
%token DATE_LITERAL
%token STRING_LITERAL
%token FLOAT_LITERAL
%token INT_LITERAL

%type <val_s> table_name db_name desc_stmt IDENTIFIER STRING_LITERAL DATE_LITERAL
%type <val_s> create_db_stmt drop_db_stmt use_db_stmt drop_tb_stmt
%type <val_s> table_join
%type <val_f> FLOAT_LITERAL
%type <t_field> fieldList
%type <ref_column> column_ref
%type <val_i> show_stmt column_type column_constraints column_constraint type_width
%type <val_i> INT_LITERAL compare_op logic_op
%type <def_column> column_decs column_dec
%type <def_table> create_tb_stmt
%type <list> expr_list value_list column_list expr_list_or_star table_refs select_expr_list
%type <list> tb_opt_decs tb_opt_exist
%type <t_constraint> tb_opt_dec
%type <insert_argu> insert_stmt table_columns
%type <expr_node> expr condition_expr condition_term where_clause
%type <select_argu> select_stmt
%type <delete_argu> delete_stmt
%type <update_argu> update_stmt
%type <ref_column> drop_idx_stmt create_idx_stmt
%type <expr_condition> expr_condition
%type <condition_tree> condition_tree

%start program

%%

program: stmt
        | program stmt
        ;

stmt: create_db_stmt ';' {execute_create_db($1);}
        | drop_db_stmt ';' {execute_drop_db($1);}
        | use_db_stmt  ';' {execute_use_db($1);}
        | show_stmt  ';' {execute_show_tables();}

        | create_tb_stmt ';' {execute_create_tb($1);}
        | drop_tb_stmt ';' {execute_drop_table($1);}
        | desc_stmt  ';' {execute_desc_tables($1);}
        | insert_stmt ';' {execute_insert_row($1);}
        | delete_stmt ';' {execute_delete($1);}
        | update_stmt ';' {execute_update($1);}
        | select_stmt ';' {execute_select($1);}

        | create_idx_stmt ';' {execute_create_idx($1);}
        | drop_idx_stmt ';' {execute_drop_idx($1);}

        | alterStmt ';' {}
        
        | EXIT ';' {execute_sql_eof(); exit(0);}
        ;

create_db_stmt: CREATE DATABASE db_name {$$=$3;}
                ;

drop_db_stmt: DROP DATABASE db_name {$$=$3;}
                ;

use_db_stmt: USE db_name {$$=$2;}
            ;

show_stmt: SHOW TABLES {}
            ;

create_tb_stmt: CREATE TABLE table_name '('fieldList')' {
                    $$ = (table_def*)malloc(sizeof(table_def));
                    $$->name = $3;
                    $$->columns = $5->columns;
                    $$->constraints = $6->constraints;
                }
                ;

drop_tb_stmt: DROP TABLE table_name {$$=$3;}
            ;

desc_stmt: DESC table_name { $$=$2; }
            ;

insert_stmt: INSERT INTO table_name VALUES value_list {
            $$ = (insert_argu*) malloc(sizeof(insert_argu));
            $$->table=$3;$$->values=$5;}
            ;
        
delete_stmt: DELETE FROM table_name where_clause {
                $$ = (delete_argu*) malloc(sizeof(delete_argu));
                $$->table = $3;
                $$->where = $4;
            }
            ;

update_stmt: UPDATE table_name  SET column_ref '=' value where_clause {
                $$ = (update_argu*) malloc(sizeof(update_argu));
                $$->table = $2;
                $$->column = $4;
                $$->val_expr = $6;
                $$->where = $7;
            }
            ;
set_clause: column_ref '=' value {}
            | set_clause',' column_ref '=' value {}

select_stmt: SELECT expr_list_or_star FROM table_refs where_clause{
                $$ = (select_argu*)malloc(sizeof(select_argu));
                $$->column_expr = $2;
                $$->tables = $4;
                $$->where = $5;
            }
            ;


create_idx_stmt: CREATE INDEX IDENTIFIER ON IDENTIFIER '(' column_list ')' {
                $$=(index_argu*)malloc(sizeof(index_argu));
                $$->index_name = $3;
                $$->table=$5;
                $$->columns=$7;}
                | ALTER TABLE IDENTIFIER ADD INDEX IDENTIFIER '(' column_list ')'{
                $$=(index_argu*)malloc(sizeof(index_argu));
                $$->index_name = $6;
                $$->table=$3;
                $->columns=$8;
                }
                ;

drop_idx_stmt: DROP INDEX IDENTIFIER{
                $$=(index_argu*)malloc(sizeof(index_argu));
                $$->index_name=$3;}
                | ALTER TABLE IDENTIFIER DROP INDEX IDENTIFIER{
                $$=(index_argu*)malloc(sizeof(index_argu));
                $$->index_name=$6;
                $$->table=$3;
                }
                | ALTER TABLE IDENTIFIER ADD INDEX IDENTIFIER’(’<column_name_list>’)’{

                }
                | ALTER TABLE IDENTIFIER DROP INDEX IDENTIFIER{

                }
                ;

alterStmt := ALTER TABLE IDENTIFIER ADD column_dec{ execute_add_column($3,$5); }
		| ALTER TABLE IDENTIFIER DROP IDENTIFIER{ execute_drop_column($3,$5); }
		| ALTER TABLE IDENTIFIER CHANGE IDENTIFIER column_dec{
            temp=(column_ref*)malloc(sizeof(column_ref));
            temp->table=$3;
            temp->column=$5;
            execute_drop_column(temp,$5);
        }
		| ALTER TABLE IDENTIFIER RENAME TO IDENTIFIER{
            execute_rename_table($3, $6);

        }
		| ALTER TABLE IDENTIFIER DROP PRIMARY KEY{
            execute_drop_primary_key($3);
        }
		| ALTER TABLE IDENTIFIER ADD CONSTRAINT IDENTIFIER tb_opt_dec {
            execute_add_constraint($3, $6, $7);
        }
        
		| ALTER TABLE IDENTIFIER DROP PRIMARY KEY IDENTIFIER{
            execute_drop_primary_key_byname($3, $7);
        }   
		
		| ALTER TABLE IDENTIFIER DROP FOREIGN KEY IDENTIFIER{
            execute_drop_foreign_key($3, $7);
        }

fieldList: column_dec {$$ = $1}
            | tb_opt_dec {$$ = $1}
            | fieldList ',' column_decs {$$->columns->next = $1; $$->columns = $3;}
            | fieldList ',' tb_opt_dec {$$->constraints->next = $1; $$->constraints = $3;}

column_dec: IDENTIFIER column_type column_constraint {
                $$ = (column_defs*)malloc(sizeof(column_defs));
                $$->name = $1;
                $$->type = $2;
                $$->constraint = $3;
                $$->next = NULL;
            }
            ;

column_constraint: NOT TOKEN_NULL {
            $$ = (column_constraint*)malloc(sizeof(column_constraint));
            $$->flags = COLUMN_FLAG_NOTNULL;
            }
            | DEFAULT value {
            $$ = (column_constraint*)malloc(sizeof(column_constraint));
            $$->flags = COLUMN_FLAG_DEFAULT;
            $$->default_value = $2;
            }
            | NOT TOKEN_NULL DEFAULT value{
            $$ = (column_constraint*)malloc(sizeof(column_constraint));
            $$->flags = COLUMN_FLAG_DEFAULT | COLUMN_FLAG_NOTNULL;
            $$->default_value = $4;
            }
            ;

tb_opt_dec: PRIMARY KEY '(' column_list ')' {
                $$=(table_constraint*)calloc(1,sizeof(table_constraint));
                $$->type = CONSTRAINT_PRIMARY_KEY;
                $$->column_list = $4;
            }
            |FOREIGN KEY '(' column_list ')' REFERENCES IDENTIFIER '(' column_list ')'{
                $$=(table_constraint*)calloc(1,sizeof(table_constraint));
                $$->type = CONSTRAINT_FOREIGN_KEY;
                $$->column_list = $4;
                $$->foreign_table_name = $7;
                $$->foreign_column_list = $9;
            }
            ;

expr_list_or_star: select_expr_list {$$=$1;}
            | '*' {$$=0;}
            ;

where_clause: WHERE condition_expr {$$=$2;}
            | {$$=NULL;}
            ;

column_type: INT   {$$=COLUMN_TYPE_INT;}
            | VARCHAR  {$$=COLUMN_TYPE_VARCHAR;}
            | FLOAT {$$=COLUMN_TYPE_FLOAT;}
            | DATE {$$=COLUMN_TYPE_DATE;}
            ;

table_refs: table_name {$$=(linked_list*)calloc(1,sizeof(linked_list));$$->data=$1;}
            | table_refs ',' table_name {$$=(linked_list*)calloc(1,sizeof(linked_list));$$->data=$3;$$->next=$1;}
            ;

value_list:  '(' expr_list ')' {$$=(linked_list*)calloc(1,sizeof(linked_list));$$->data=$2;}
           | value_list ','  '(' expr_list ')'  {$$=(linked_list*)calloc(1,sizeof(linked_list));$$->data=$4;$$->next=$1;}
           ;

expr_list:  value {$$=(linked_list*)calloc(1,sizeof(linked_list));$$->data=$1;}
           | expr_list ','  value  {$$=(linked_list*)calloc(1,sizeof(linked_list));$$->data=$3;$$->next=$1;}
           ;

select_expr_list:  column_ref {
                        $$ = (linked_list)calloc(1,sizeof(linked_list));
                        $$->data = $1;
                    }
                |select_expr_list',' column_ref{
                        $1->next = $3;
                        $$ = $1;
                    }

table_columns: table_name {$$=(insert_argu*)calloc(1,sizeof(insert_argu));$$->table=$1;}
            | table_name '(' column_list ')' {$$=(insert_argu*)calloc(1,sizeof(insert_argu));$$->table=$1;$$->columns=$3;}
            ;

column_list: column_ref {$$=(linked_list*)calloc(1,sizeof(linked_list));$$->data=$1;}
           | column_list ','  column_ref  {$$=(linked_list*)calloc(1,sizeof(linked_list));$$->data=$3;$$->next=$1;}
           ;

condition_expr: condition_term {
            $$=(condition_tree*)calloc(1,sizeof(condition_tree));
            $$->node = $1;
            }
            | condition_expr logic_op condition_expr {
                $$=(condition_tree*)calloc(1,sizeof(condition_tree));
                $$->left=$1;
                $$->right=$3;
                $$->op=$2;
            }
            ;

logic_op: AND { $$ = OPER_AND; }
        | OR { $$ = OPER_OR; }
        ;

condition_term: column_ref compare_op value {
                $$=(expr_condition*)calloc(1,sizeof(expr_condition));
                $$->column=$1;
                $$->value=$3;
                $$->op=$3;
            }
            | column_ref IS NULL{
                $$=(expr_condition*)calloc(1,sizeof(expr_condition));
                $$->column=$1;
                $$->op=OPER_ISNULL;
            }
            | column_ref IS NOT NULL{
                $$=(expr_condition*)calloc(1,sizeof(expr_condition));
                $$->column=$1;
                $$->op=OPER_NOTNULL;
            }
            ;

compare_op: '=' {$$ = OPER_EQU;}
            | '>' {$$=OPER_GT;}
            | '<' {$$=OPER_LT;}
            | GEQ {$$=OPER_GE;}
            | LEQ {$$=OPER_LE;}
            | NEQ {$$=OPER_NEQ;}
            | LIKE {$$=OPER_LIKE;}
            ;
value: INT_LITERAL {
            $$=(expr_node*)calloc(1,sizeof(expr_node));
            $$->literal_i=$1;
            $$->node_type=TERM_INT;
        }
    | FLOAT_LITERAL {
            $$=(expr_node*)calloc(1,sizeof(expr_node));
            $$->literal_f=$1;
            $$->node_type=TERM_FLOAT;
        }
    | DATE_LITERAL {
            $$=(expr_node*)calloc(1,sizeof(expr_node));
            $$->literal_s=$1;
            $$->node_type=TERM_DATE;
        }
    | STRING_LITERAL {
            $$=(expr_node*)calloc(1,sizeof(expr_node));
            $$->literal_s=$1;
            $$->node_type=TERM_STRING;
        }
    | TOKEN_NULL {
            $$=(expr_node*)calloc(1,sizeof(expr_node));
            $$->node_type=TERM_NULL;
        }

expr: column_ref{
            $$=(expr_node*)calloc(1,sizeof(expr_node));
            $$->column=$1;
            $$->node_type=TERM_COLUMN;
        }
    | value{
        $$ = $1;
    }

column_name_list: IDENTIFIER, column_name_list{ temp=(linked_list*)calloc(1,sizeof(linked_list));$$->data = temp; $3->next = temp; $$ = $3}
    | IDENTIFIER{$$=(linked_list*)calloc(1,sizeof(linked_list));$$->data = $1; $$->next = null;}

column_ref: IDENTIFIER {$$=(column_ref*)calloc(1,sizeof(column_ref));$$->table = NULL; $$->column = $1;};
    | table_name '.' IDENTIFIER {$$=(column_ref*)calloc(1,sizeof(column_ref));$$->table = $1; $$->column = $3; };
    ;

col: [table_name'.']column_name{ }

table_name: IDENTIFIER {$$ = $1;}
        ;

db_name: IDENTIFIER { $$=$1; }
        ;

%%

int yyerror(const char *str)
{
    fprintf(stderr, "Error: %s\n", str);
    return 1;
}
int yywrap()
{
    return 1;
}
char start_parse(const char *expr_input)
{
    char ret;
    if(expr_input){
        YY_BUFFER_STATE my_string_buffer = yy_scan_string(expr_input);
        yy_switch_to_buffer( my_string_buffer ); // switch flex to the buffer we just created
        ret = yyparse();
        yy_delete_buffer(my_string_buffer );
    }else{
        ret = yyparse();
    }
    execute_sql_eof();
    return ret;
}
