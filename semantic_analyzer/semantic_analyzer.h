#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 数据类型 */
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_CHAR,
    TYPE_VOID,
    TYPE_UNKNOWN,
    TYPE_ERROR
} DataType;

/* 符号表项 */
typedef struct Symbol {
    char *name;
    DataType type;
    int scope_level;
    int line_number;
    struct Symbol *next;
} Symbol;

/* 作用域栈 */
typedef struct ScopeStack {
    Symbol *symbols[100];
    int scope_count;
} ScopeStack;

/* AST 节点 */
typedef struct ASTNode {
    char *label;
    int child_count;
    struct ASTNode **children;
} ASTNode;

/* 函数声明 */
Symbol* symbol_lookup(ScopeStack *stack, const char *name);
Symbol* symbol_insert(ScopeStack *stack, const char *name, DataType type, int line);
void scope_push(ScopeStack *stack);
void scope_pop(ScopeStack *stack);
void symbol_table_init(ScopeStack *stack);
void symbol_table_free(ScopeStack *stack);
void symbol_table_print(ScopeStack *stack);

DataType type_from_string(const char *type_str);
const char* type_to_string(DataType type);
int types_compatible(DataType t1, DataType t2);

void semantic_analysis(ASTNode *node, ScopeStack *stack);

extern int semantic_error_count;
extern int semantic_warning_count;

void semantic_error(int line, const char *format, ...);
void semantic_warning(int line, const char *format, ...);

#endif
