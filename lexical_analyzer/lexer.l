%option noyywrap yylineno nodefault

%{
#include <stdio.h>
#include <string.h>

static void print_token(const char *type) {
    printf("%-12s  line %-4d  %s\n", type, yylineno, yytext);
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

"if"           { print_token("KW_IF"); }
"else"         { print_token("KW_ELSE"); }
"while"        { print_token("KW_WHILE"); }
"for"          { print_token("KW_FOR"); }
"return"       { print_token("KW_RETURN"); }
"int"          { print_token("KW_INT"); }
"float"        { print_token("KW_FLOAT"); }
"char"         { print_token("KW_CHAR"); }
"void"         { print_token("KW_VOID"); }
"break"        { print_token("KW_BREAK"); }
"continue"     { print_token("KW_CONTINUE"); }
"struct"       { print_token("KW_STRUCT"); }

{FLOAT}         { print_token("FLOAT_CONST"); }
{INT}           { print_token("INT_CONST"); }
{STRING}        { print_token("STRING_LITERAL"); }
{CHAR}          { print_token("CHAR_LITERAL"); }
{ID}            { print_token("IDENTIFIER"); }

"=="|"!="|"<="|">="|"&&"|"||" { print_token("OPERATOR"); }
"++"|"--"|"+"|"-"|"*"|"/"|"%"|"="|"<"|">"|"!" { print_token("OPERATOR"); }
";"|","|"("|")"|"{"|"}"|"["|"]" { print_token("DELIMITER"); }

"//"[^\n]*     { /* skip single-line comment */ }
\/\*([^*]|\*+[^*/])*\*+\/  { /* skip block comment */ }
[ \t\r\n]+     { /* skip whitespace */ }

.               {
                    fprintf(stderr, "Lexical error at line %d: unexpected character '%s'\n", yylineno, yytext);
                }

%%

int main(void) {
    puts("Enter source text. Press Ctrl+Z then Enter to finish.");
    yylex();
    return 0;
}
