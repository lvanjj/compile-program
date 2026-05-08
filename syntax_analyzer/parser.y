%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../semantic_analyzer/semantic_analyzer.h"

int yylex(void);
void yyerror(const char *s);
extern int yylineno;
extern char *yytext;
extern FILE *yyin;

#define YYERROR_VERBOSE 1

static ASTNode *ast_new(const char *label);
static ASTNode *ast_new_token(const char *kind, const char *lexeme);
static void ast_add_child(ASTNode *parent, ASTNode *child);
static void ast_print(ASTNode *node, int indent);
static void ast_free(ASTNode *node);

ASTNode *syntax_root = NULL;
%}

%code requires {
  #include "../semantic_analyzer/semantic_analyzer.h"
}

%union {
    char *sval;
    ASTNode *node;
}

%token <sval> INT_CONST FLOAT_CONST STRING_LITERAL CHAR_LITERAL IDENTIFIER
%token KW_IF KW_ELSE KW_WHILE KW_FOR KW_RETURN KW_INT KW_FLOAT KW_CHAR KW_VOID KW_BREAK KW_CONTINUE KW_STRUCT
%token OR AND EQ NE LE GE LT GT NOT ASSIGN

%type <node> program external_list external declaration type_specifier init_declarator_list init_declarator statement compound_statement statement_list_opt statement_list expression_statement selection_statement iteration_statement expression_opt jump_statement expression assignment_expression logical_or_expression logical_and_expression equality_expression relational_expression additive_expression multiplicative_expression unary_expression primary_expression

%left '+' '-'
%left '*' '/' '%'
%left OR
%left AND
%left EQ NE
%left LT LE GT GE
%right ASSIGN
%right UMINUS
%nonassoc LOWER_THAN_ELSE
%nonassoc KW_ELSE

%%

program
    : external_list
      {
          $$ = ast_new("Program");
          ast_add_child($$, $1);
          syntax_root = $$;
      }
    ;

external_list
    : external_list external
      {
          $$ = $1;
          ast_add_child($$, $2);
      }
    | external
      {
          $$ = ast_new("TranslationUnit");
          ast_add_child($$, $1);
      }
    ;

external
    : declaration
      { $$ = $1; }
    | statement
      { $$ = $1; }
    ;

declaration
    : type_specifier init_declarator_list ';'
      {
          $$ = ast_new("Declaration");
          ast_add_child($$, $1);
          ast_add_child($$, $2);
      }
    | type_specifier error ';'
      {
          yyerror("invalid declaration");
          yyerrok;
          $$ = ast_new("Declaration<error>");
          ast_add_child($$, $1);
      }
    ;

type_specifier
    : KW_INT
      { $$ = ast_new("Type:int"); }
    | KW_FLOAT
      { $$ = ast_new("Type:float"); }
    | KW_CHAR
      { $$ = ast_new("Type:char"); }
    | KW_VOID
      { $$ = ast_new("Type:void"); }
    | KW_STRUCT IDENTIFIER
      {
          $$ = ast_new("Type:struct");
          ast_add_child($$, ast_new_token("Identifier", $2));
          free($2);
      }
    ;

init_declarator_list
    : init_declarator_list ',' init_declarator
      {
          $$ = $1;
          ast_add_child($$, $3);
      }
    | init_declarator
      {
          $$ = ast_new("InitDeclaratorList");
          ast_add_child($$, $1);
      }
    ;

init_declarator
    : IDENTIFIER
      {
          $$ = ast_new_token("Declarator", $1);
          free($1);
      }
    | IDENTIFIER ASSIGN expression
      {
          $$ = ast_new("InitDeclarator");
          ast_add_child($$, ast_new_token("Identifier", $1));
          free($1);
          ast_add_child($$, $3);
      }
    ;

statement
    : compound_statement { $$ = $1; }
    | expression_statement { $$ = $1; }
    | selection_statement { $$ = $1; }
    | iteration_statement { $$ = $1; }
    | jump_statement { $$ = $1; }
    ;

compound_statement
    : '{' statement_list_opt '}'
      {
          $$ = ast_new("CompoundStatement");
          ast_add_child($$, $2);
      }
    | '{' error '}'
      {
          yyerror("invalid compound statement");
          yyerrok;
          $$ = ast_new("CompoundStatement<error>");
      }
    ;

statement_list_opt
    : statement_list { $$ = $1; }
    | /* empty */ { $$ = ast_new("StatementList<empty>"); }
    ;

statement_list
    : statement_list statement
      {
          $$ = $1;
          ast_add_child($$, $2);
      }
    | statement
      {
          $$ = ast_new("StatementList");
          ast_add_child($$, $1);
      }
    ;

expression_statement
    : ';'
      { $$ = ast_new("EmptyExpressionStatement"); }
    | expression ';'
      {
          $$ = ast_new("ExpressionStatement");
          ast_add_child($$, $1);
      }
    | error ';'
      {
          yyerror("invalid expression statement");
          yyerrok;
          $$ = ast_new("ExpressionStatement<error>");
      }
    ;

selection_statement
    : KW_IF '(' expression ')' statement %prec LOWER_THAN_ELSE
      {
          $$ = ast_new("IfStatement");
          ast_add_child($$, $3);
          ast_add_child($$, $5);
      }
    | KW_IF '(' expression ')' statement KW_ELSE statement
      {
          $$ = ast_new("IfElseStatement");
          ast_add_child($$, $3);
          ast_add_child($$, $5);
          ast_add_child($$, $7);
      }
    | KW_IF '(' error ')' statement
      {
          yyerror("invalid if condition");
          yyerrok;
          $$ = ast_new("IfStatement<error>");
          ast_add_child($$, $5);
      }
    ;

iteration_statement
    : KW_WHILE '(' expression ')' statement
      {
          $$ = ast_new("WhileStatement");
          ast_add_child($$, $3);
          ast_add_child($$, $5);
      }
    | KW_WHILE '(' error ')' statement
      {
          yyerror("invalid while condition");
          yyerrok;
          $$ = ast_new("WhileStatement<error>");
          ast_add_child($$, $5);
      }
    | KW_FOR '(' expression_statement expression_statement expression_opt ')' statement
      {
          $$ = ast_new("ForStatement");
          ast_add_child($$, $3);
          ast_add_child($$, $4);
          ast_add_child($$, $5);
          ast_add_child($$, $7);
      }
    | KW_FOR '(' error ')' statement
      {
          yyerror("invalid for clause");
          yyerrok;
          $$ = ast_new("ForStatement<error>");
          ast_add_child($$, $5);
      }
    ;

expression_opt
    : expression { $$ = $1; }
    | /* empty */ { $$ = ast_new("Expression<empty>"); }
    ;

jump_statement
    : KW_RETURN expression_opt ';'
      {
          $$ = ast_new("ReturnStatement");
          ast_add_child($$, $2);
      }
    | KW_BREAK ';'
      { $$ = ast_new("BreakStatement"); }
    | KW_CONTINUE ';'
      { $$ = ast_new("ContinueStatement"); }
    ;

expression
    : assignment_expression { $$ = $1; }
    ;

assignment_expression
    : logical_or_expression { $$ = $1; }
    | IDENTIFIER ASSIGN assignment_expression %prec ASSIGN
      {
          $$ = ast_new("Assign");
          ast_add_child($$, ast_new_token("Identifier", $1));
          free($1);
          ast_add_child($$, $3);
      }
    ;

logical_or_expression
    : logical_or_expression OR logical_and_expression
      {
          $$ = ast_new("LogicalOr");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | logical_and_expression { $$ = $1; }
    ;

logical_and_expression
    : logical_and_expression AND equality_expression
      {
          $$ = ast_new("LogicalAnd");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | equality_expression { $$ = $1; }
    ;

equality_expression
    : equality_expression EQ relational_expression
      {
          $$ = ast_new("Equal");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | equality_expression NE relational_expression
      {
          $$ = ast_new("NotEqual");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | relational_expression { $$ = $1; }
    ;

relational_expression
    : relational_expression LT additive_expression
      {
          $$ = ast_new("LessThan");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | relational_expression LE additive_expression
      {
          $$ = ast_new("LessEqual");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | relational_expression GT additive_expression
      {
          $$ = ast_new("GreaterThan");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | relational_expression GE additive_expression
      {
          $$ = ast_new("GreaterEqual");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | additive_expression { $$ = $1; }
    ;

additive_expression
    : additive_expression '+' multiplicative_expression
      {
          $$ = ast_new("Add");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | additive_expression '-' multiplicative_expression
      {
          $$ = ast_new("Sub");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | multiplicative_expression { $$ = $1; }
    ;

multiplicative_expression
    : multiplicative_expression '*' unary_expression
      {
          $$ = ast_new("Mul");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | multiplicative_expression '/' unary_expression
      {
          $$ = ast_new("Div");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | multiplicative_expression '%' unary_expression
      {
          $$ = ast_new("Mod");
          ast_add_child($$, $1);
          ast_add_child($$, $3);
      }
    | unary_expression { $$ = $1; }
    ;

unary_expression
    : primary_expression { $$ = $1; }
    | '-' unary_expression %prec UMINUS
      {
          $$ = ast_new("Negate");
          ast_add_child($$, $2);
      }
    | NOT unary_expression
      {
          $$ = ast_new("Not");
          ast_add_child($$, $2);
      }
    ;

primary_expression
    : IDENTIFIER
      {
          $$ = ast_new_token("Identifier", $1);
          free($1);
      }
    | INT_CONST
      {
          $$ = ast_new_token("IntConst", $1);
          free($1);
      }
    | FLOAT_CONST
      {
          $$ = ast_new_token("FloatConst", $1);
          free($1);
      }
    | STRING_LITERAL
      {
          $$ = ast_new_token("StringLiteral", $1);
          free($1);
      }
    | CHAR_LITERAL
      {
          $$ = ast_new_token("CharLiteral", $1);
          free($1);
      }
    | '(' expression ')'
      { $$ = $2; }
    ;

%%

ASTNode *syntax_get_root(void) {
  return syntax_root;
}

#ifndef SYNTAX_NO_MAIN
int main(int argc, char **argv) {
    FILE *input_file = NULL;
    puts("Enter source text. Press Ctrl+Z then Enter to finish.");
    if (argc > 1) {
        input_file = fopen(argv[1], "r");
        if (input_file == NULL) {
            perror(argv[1]);
            return 1;
        }
        yyin = input_file;
    }
    if (yyparse() == 0) {
        puts("Syntax analysis succeeded.");
        puts("Syntax tree:");
      ast_print(syntax_root, 0);
      ast_free(syntax_root);
        if (input_file != NULL) {
            fclose(input_file);
        }
        return 0;
    }
    if (input_file != NULL) {
        fclose(input_file);
    }
    puts("Syntax analysis failed.");
    return 1;
}
#endif

void yyerror(const char *s) {
    const char *token_text = (yytext != NULL && yytext[0] != '\0') ? yytext : "EOF";
    fprintf(stderr, "Syntax error at line %d near '%s': %s\n", yylineno, token_text, s);
}

static char *dup_text(const char *text) {
    size_t length = strlen(text) + 1;
    char *copy = (char *)malloc(length);
    if (copy != NULL) {
        memcpy(copy, text, length);
    }
    return copy;
}

static ASTNode *ast_new(const char *label) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (node == NULL) {
        return NULL;
    }
    node->label = dup_text(label);
    node->child_count = 0;
    node->children = NULL;
    return node;
}

static ASTNode *ast_new_token(const char *kind, const char *lexeme) {
    size_t kind_len = strlen(kind);
    size_t lexeme_len = strlen(lexeme);
    char *label = (char *)malloc(kind_len + lexeme_len + 2);
    ASTNode *node;
    if (label == NULL) {
        return NULL;
    }
    memcpy(label, kind, kind_len);
    label[kind_len] = ':';
    memcpy(label + kind_len + 1, lexeme, lexeme_len + 1);
    node = ast_new(label);
    free(label);
    return node;
}

static void ast_add_child(ASTNode *parent, ASTNode *child) {
    ASTNode **children;
    if (parent == NULL || child == NULL) {
        return;
    }
    children = (ASTNode **)realloc(parent->children, sizeof(ASTNode *) * (parent->child_count + 1));
    if (children == NULL) {
        return;
    }
    parent->children = children;
    parent->children[parent->child_count] = child;
    parent->child_count += 1;
}

static void ast_print_indent(int indent) {
    int index;
    for (index = 0; index < indent; ++index) {
        putchar(' ');
    }
}

static void ast_print(ASTNode *node, int indent) {
    int index;
    if (node == NULL) {
        return;
    }
    ast_print_indent(indent);
    printf("%s\n", node->label != NULL ? node->label : "<null>");
    for (index = 0; index < node->child_count; ++index) {
        ast_print(node->children[index], indent + 2);
    }
}

static void ast_free(ASTNode *node) {
    int index;
    if (node == NULL) {
        return;
    }
    for (index = 0; index < node->child_count; ++index) {
        ast_free(node->children[index]);
    }
    free(node->children);
    free(node->label);
    free(node);
}
