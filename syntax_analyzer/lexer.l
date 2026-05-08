%option noyywrap yylineno nodefault noinput nounput

%{
#define YY_NO_INPUT 1
#define YY_NO_UNPUT 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"

static char *dup_text(const char *text) {
    size_t length = strlen(text) + 1;
    char *copy = (char *)malloc(length);
    if (copy != NULL) {
        memcpy(copy, text, length);
    }
    return copy;
}
%}

DIGIT      [0-9]
LETTER     [A-Za-z_]
ID         {LETTER}({LETTER}|{DIGIT})*
INT        {DIGIT}+
FLOAT      ({DIGIT}+\.{DIGIT}*|\.{DIGIT}+)([eE][+-]?{DIGIT}+)?|{DIGIT}+[eE][+-]?{DIGIT}+
STRING     \"([^\"\\\n]|\\.)*\"
CHAR       \'([^\'\\\n]|\\.)\'

%%

"if"           { return KW_IF; }
"else"         { return KW_ELSE; }
"while"        { return KW_WHILE; }
"for"          { return KW_FOR; }
"return"       { return KW_RETURN; }
"int"          { return KW_INT; }
"float"        { return KW_FLOAT; }
"char"         { return KW_CHAR; }
"void"         { return KW_VOID; }
"break"        { return KW_BREAK; }
"continue"     { return KW_CONTINUE; }
"struct"       { return KW_STRUCT; }

{FLOAT}         { yylval.sval = dup_text(yytext); return FLOAT_CONST; }
{INT}           { yylval.sval = dup_text(yytext); return INT_CONST; }
{STRING}        { yylval.sval = dup_text(yytext); return STRING_LITERAL; }
{CHAR}          { yylval.sval = dup_text(yytext); return CHAR_LITERAL; }
{ID}            { yylval.sval = dup_text(yytext); return IDENTIFIER; }

"=="           { return EQ; }
"!="           { return NE; }
"<="           { return LE; }
">="           { return GE; }
"&&"           { return AND; }
"||"           { return OR; }
"<"            { return LT; }
">"            { return GT; }
"!"            { return NOT; }
"="            { return ASSIGN; }
"+"|"-"|"*"|"/"|"%" { return yytext[0]; }
";"|","|"("|")"|"{"|"}"|"["|"]" { return yytext[0]; }

"//"[^\n]*     { /* skip single-line comment */ }
\xEF\xBB\xBF   { /* skip UTF-8 BOM */ }
\/\*([^*]|\*+[^*/])*\*+\/  { /* skip block comment */ }
[ \t\r\n]+     { /* skip whitespace */ }

.               {
                    fprintf(stderr, "Lexical error at line %d: unexpected character '%s'\n", yylineno, yytext);
                    return yytext[0];
                }

%%
