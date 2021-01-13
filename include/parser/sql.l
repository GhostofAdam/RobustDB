/*
Author: Harry Chen
Original Author: Yuxiang Zhang
Inspired by: https://raw.githubusercontent.com/thinkpad20/sql/master/src/lex/sql.l
*/

%{
#include <ctype.h>
#include "parser/type_def.hpp"
#include "sql.tab.h"

#ifndef __linux__
char *strndup (const char *s, size_t n)
{
  char *result;
  size_t len = strlen(s);

  if (n < len)
    len = n;

  result = (char *) malloc(len + 1);
  if (!result)
    return 0;

  result[len] = '\0';
  return (char *)memcpy(result, s, len);
}
#endif

%}

DATE_FORMAT     '[0-9]+-[0-9]+-[0-9]+'
ID_FORMAT       [a-zA-Z][a-zA-Z0-9_]*
STRING_FORMAT   '(\\\\|\\n|\\r|\\\"|\\0|\\'|[^'\\])*'
INT_FORMAT      [0-9]+
FLOAT_FORMAT    [0-9]+|([0-9]*\.[0-9]+)([eE][-+]?[0-9]+)?

%%

create|CREATE                       { return CREATE; }
table|TABLE                         { return TABLE; }
database|DATABASE                   { return DATABASE; }
drop|DROP                           { return DROP; }
use|USE                             { return USE; }
update|UPDATE                       { return UPDATE; }
show|SHOW                           { return SHOW; }
insert|INSERT                       { return INSERT; }
alter|ALTER                         { return ALTER; }
into|INTO                           { return INTO; }
select|SELECT                       { return SELECT; }
delete|DELETE                       { return DELETE; }
from|FROM                           { return FROM; }
set|SET                             { return SET; }
where|WHERE                         { return WHERE; }
primary|PRIMARY                     { return PRIMARY; }
foreign|FOREIGN                     { return FOREIGN; }
references|REFERENCES               { return REFERENCES; }
key|KEY                             { return KEY; }
index|INDEX                         { return INDEX; }
default|DEFAULT                     { return DEFAULT; }
check|CHECK                         { return CHECK; }
not|NOT                             { return NOT; }
null|NULL                           { return TOKEN_NULL; }
and|AND                             { return AND; }
or|OR                               { return OR; }
"!="                                { return NEQ; }
"<>"                                { return NEQ; }
">="                                { return GEQ; }
"<="                                { return LEQ; }
int|INT                             { return INT; }
integer|INTEGER                     { return INT; }
double|DOUBLE                       { return DOUBLE; }
float|FLOAT                         { return FLOAT; }
char|CHAR                           { return CHAR; }
varchar|VARCHAR                     { return VARCHAR; }
date|DATE                           { return DATE; }
full|FULL                           { return FULL; }
values|VALUES                       { return VALUES; }
desc|DESC                           { return DESC; }
is|IS                               { return IS; }
on|ON                               { return ON; }
true|TRUE                           { return TRUE; }
false|FALSE                         { return FALSE; }
exit|EXIT                           { return EXIT; }
{ID_FORMAT}                         { yylval.val_s = strdup(yytext); return IDENTIFIER; }
{DATE_FORMAT}                       { yylval.val_s = strndup(yytext + 1, strlen(yytext) - 2); return DATE_LITERAL;}
{STRING_FORMAT}                     { yylval.val_s = strndup(yytext + 1, strlen(yytext) - 2); return STRING_LITERAL; }
{INT_FORMAT}                        { yylval.val_i = atoi(yytext); return INT_LITERAL; }
{FLOAT_FORMAT}                      { yylval.val_f = atof(yytext); return FLOAT_LITERAL; }
[ \t\r\n]+                          { /* ignore */ }
.                                   { return(yytext[0]);}
%%
