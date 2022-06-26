#pragma once
#include "mainstruct.h"

extern int yylex(void); // lexan.l -- lexer inteface
extern FILE *yyin; // laxan.l

extern int yyparse(void); //syntan.y -- parser interface
extern int yyerror(const char*  s); // syntax.y
extern VS_AsnCompilerContainer world; // syntax.y

