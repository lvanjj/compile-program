#include "semantic_analyzer.h"
#include <stdarg.h>

int semantic_error_count = 0;
int semantic_warning_count = 0;

/* ========== 工具函数 ========== */

DataType type_from_string(const char *type_str) {
    if (type_str == NULL) return TYPE_UNKNOWN;
    if (strstr(type_str, "int") != NULL) return TYPE_INT;
    if (strstr(type_str, "float") != NULL) return TYPE_FLOAT;
    if (strstr(type_str, "char") != NULL) return TYPE_CHAR;
    if (strstr(type_str, "void") != NULL) return TYPE_VOID;
    return TYPE_UNKNOWN;
}

const char* type_to_string(DataType type) {
    switch (type) {
        case TYPE_INT:     return "int";
        case TYPE_FLOAT:   return "float";
        case TYPE_CHAR:    return "char";
        case TYPE_VOID:    return "void";
        case TYPE_UNKNOWN: return "unknown";
        case TYPE_ERROR:   return "error";
        default:           return "?";
    }
}

int types_compatible(DataType t1, DataType t2) {
    if (t1 == t2) return 1;
    if (t1 == TYPE_ERROR || t2 == TYPE_ERROR) return 1;
    if ((t1 == TYPE_INT && t2 == TYPE_FLOAT) || 
        (t1 == TYPE_FLOAT && t2 == TYPE_INT)) {
        return 1;
    }
    return 0;
}

/* ========== 错误处理 ========== */

void semantic_error(int line, const char *format, ...) {
    va_list args;
    semantic_error_count++;
    fprintf(stderr, "Semantic Error at line %d: ", line);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void semantic_warning(int line, const char *format, ...) {
    va_list args;
    semantic_warning_count++;
    fprintf(stderr, "Semantic Warning at line %d: ", line);
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/* ========== 符号表操作 ========== */

void symbol_table_init(ScopeStack *stack) {
    stack->scope_count = 0;
    scope_push(stack);
}

void symbol_table_free(ScopeStack *stack) {
    int i;
    for (i = 0; i < stack->scope_count; i++) {
        Symbol *sym = stack->symbols[i];
        while (sym != NULL) {
            Symbol *next = sym->next;
            free(sym->name);
            free(sym);
            sym = next;
        }
    }
    stack->scope_count = 0;
}

void scope_push(ScopeStack *stack) {
    if (stack->scope_count < 100) {
        stack->symbols[stack->scope_count] = NULL;
        stack->scope_count++;
    }
}

void scope_pop(ScopeStack *stack) {
    if (stack->scope_count > 0) {
        Symbol *sym = stack->symbols[stack->scope_count - 1];
        while (sym != NULL) {
            Symbol *next = sym->next;
            free(sym->name);
            free(sym);
            sym = next;
        }
        stack->scope_count--;
    }
}

Symbol* symbol_lookup_current(ScopeStack *stack, const char *name) {
    if (stack->scope_count == 0 || name == NULL) return NULL;
    Symbol *sym = stack->symbols[stack->scope_count - 1];
    while (sym != NULL) {
        if (strcmp(sym->name, name) == 0) return sym;
        sym = sym->next;
    }
    return NULL;
}

Symbol* symbol_lookup(ScopeStack *stack, const char *name) {
    int i;
    if (name == NULL) return NULL;
    for (i = stack->scope_count - 1; i >= 0; i--) {
        Symbol *sym = stack->symbols[i];
        while (sym != NULL) {
            if (strcmp(sym->name, name) == 0) return sym;
            sym = sym->next;
        }
    }
    return NULL;
}

Symbol* symbol_insert(ScopeStack *stack, const char *name, DataType type, int line) {
    Symbol *existing, *new_sym;
    if (stack->scope_count == 0 || name == NULL) return NULL;
    
    existing = symbol_lookup_current(stack, name);
    if (existing != NULL) {
        semantic_error(line, "Symbol '%s' already declared at line %d", name, existing->line_number);
        return existing;
    }
    
    new_sym = (Symbol *)malloc(sizeof(Symbol));
    new_sym->name = (char *)malloc(strlen(name) + 1);
    strcpy(new_sym->name, name);
    new_sym->type = type;
    new_sym->scope_level = stack->scope_count - 1;
    new_sym->line_number = line;
    new_sym->next = stack->symbols[stack->scope_count - 1];
    stack->symbols[stack->scope_count - 1] = new_sym;
    return new_sym;
}

void symbol_table_print(ScopeStack *stack) {
    int i;
    printf("\n========== Symbol Table ==========\n");
    for (i = 0; i < stack->scope_count; i++) {
        printf("Scope %d:\n", i);
        Symbol *sym = stack->symbols[i];
        if (sym == NULL) {
            printf("  (empty)\n");
        }
        while (sym != NULL) {
            printf("  %-20s: %-8s (line %d)\n", sym->name, type_to_string(sym->type), sym->line_number);
            sym = sym->next;
        }
    }
    printf("==================================\n\n");
}

/* ========== AST 遍历和语义检查 ========== */

static const char* get_node_type(ASTNode *node) {
    if (node == NULL || node->label == NULL) return "";
    return node->label;
}

static int label_contains(const char *label, const char *substr) {
    if (label == NULL || substr == NULL) return 0;
    return strstr(label, substr) != NULL;
}

static char* extract_identifier_name(const char *label) {
    if (label == NULL) return NULL;
    const char *colon = strchr(label, ':');
    if (colon != NULL && *(colon + 1) != '\0') {
        return (char *)(colon + 1);
    }
    return NULL;
}

/* 前向声明 */
static DataType semantic_check_expression(ASTNode *node, ScopeStack *stack);
static void semantic_check_statement(ASTNode *node, ScopeStack *stack);

static DataType semantic_check_declaration(ASTNode *node, ScopeStack *stack) {
    ASTNode *type_node, *declarator_list;
    int i;
    DataType var_type = TYPE_UNKNOWN;
    
    if (node == NULL || node->child_count < 2) return TYPE_ERROR;
    
    type_node = node->children[0];
    if (type_node != NULL) {
        var_type = type_from_string(get_node_type(type_node));
    }
    
    declarator_list = node->children[1];
    if (declarator_list != NULL) {
        for (i = 0; i < declarator_list->child_count; i++) {
            ASTNode *declarator = declarator_list->children[i];
            if (declarator != NULL) {
                const char *label = get_node_type(declarator);
                if (label_contains(label, "Declarator:")) {
                    char *var_name = extract_identifier_name(label);
                    if (var_name != NULL) {
                        symbol_insert(stack, var_name, var_type, 0);
                    }
                } else if (label_contains(label, "InitDeclarator")) {
                    if (declarator->child_count >= 1) {
                        ASTNode *id_node = declarator->children[0];
                        if (id_node != NULL) {
                            const char *id_label = get_node_type(id_node);
                            if (label_contains(id_label, "Identifier:")) {
                                char *var_name = extract_identifier_name(id_label);
                                if (var_name != NULL) {
                                    symbol_insert(stack, var_name, var_type, 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return var_type;
}

static DataType semantic_check_assignment(ASTNode *node, ScopeStack *stack) {
    char *var_name;
    Symbol *sym;
    DataType rhs_type;
    
    if (node == NULL || node->child_count < 2) return TYPE_ERROR;
    ASTNode *id_node = node->children[0];
    if (id_node == NULL) return TYPE_ERROR;
    
    const char *id_label = get_node_type(id_node);
    if (!label_contains(id_label, "Identifier:")) return TYPE_ERROR;
    
    var_name = extract_identifier_name(id_label);
    if (var_name == NULL) return TYPE_ERROR;
    
    sym = symbol_lookup(stack, var_name);
    if (sym == NULL) {
        semantic_error(0, "Undeclared identifier '%s'", var_name);
        return TYPE_ERROR;
    }
    
    ASTNode *rhs_node = node->children[1];
    rhs_type = semantic_check_expression(rhs_node, stack);
    
    if (!types_compatible(sym->type, rhs_type)) {
        semantic_warning(0, "Type mismatch in assignment: %s = %s", 
                        type_to_string(sym->type), type_to_string(rhs_type));
    }
    
    return sym->type;
}

static DataType semantic_check_expression(ASTNode *node, ScopeStack *stack) {
    const char *label;
    
    if (node == NULL) return TYPE_ERROR;
    label = get_node_type(node);
    
    /* 二元操作 */
    if (label_contains(label, "Add") || label_contains(label, "Sub") ||
        label_contains(label, "Mul") || label_contains(label, "Div") ||
        label_contains(label, "Mod")) {
        DataType left_type = TYPE_UNKNOWN;
        DataType right_type = TYPE_UNKNOWN;
        if (node->child_count >= 2) {
            left_type = semantic_check_expression(node->children[0], stack);
            right_type = semantic_check_expression(node->children[1], stack);
            if (!types_compatible(left_type, right_type)) {
                semantic_warning(0, "Type mismatch in arithmetic operation");
            }
        }
        return left_type;
    }
    
    /* 比较和逻辑运算 */
    if (label_contains(label, "Equal") || label_contains(label, "NotEqual") ||
        label_contains(label, "LessThan") || label_contains(label, "LessEqual") ||
        label_contains(label, "GreaterThan") || label_contains(label, "GreaterEqual") ||
        label_contains(label, "LogicalAnd") || label_contains(label, "LogicalOr")) {
        if (node->child_count >= 2) {
            semantic_check_expression(node->children[0], stack);
            semantic_check_expression(node->children[1], stack);
        }
        return TYPE_INT;
    }
    
    /* 一元操作 */
    if (label_contains(label, "Negate") || label_contains(label, "Not")) {
        if (node->child_count >= 1) {
            return semantic_check_expression(node->children[0], stack);
        }
    }
    
    /* 赋值 */
    if (label_contains(label, "Assign")) {
        return semantic_check_assignment(node, stack);
    }
    
    /* 标识符 */
    if (label_contains(label, "Identifier:")) {
        char *var_name = extract_identifier_name(label);
        if (var_name != NULL) {
            Symbol *sym = symbol_lookup(stack, var_name);
            if (sym == NULL) {
                semantic_error(0, "Undeclared identifier '%s'", var_name);
                return TYPE_ERROR;
            }
            return sym->type;
        }
    }
    
    /* 常量 */
    if (label_contains(label, "IntConst:")) return TYPE_INT;
    if (label_contains(label, "FloatConst:")) return TYPE_FLOAT;
    if (label_contains(label, "CharLiteral:")) return TYPE_CHAR;
    if (label_contains(label, "StringLiteral:")) return TYPE_INT;
    
    if (node->child_count > 0) {
        return semantic_check_expression(node->children[0], stack);
    }
    
    return TYPE_UNKNOWN;
}

static void semantic_check_statement(ASTNode *node, ScopeStack *stack) {
    const char *label;
    int i;
    
    if (node == NULL) return;
    label = get_node_type(node);
    
    /* 复合语句 */
    if (label_contains(label, "CompoundStatement")) {
        scope_push(stack);
        for (i = 0; i < node->child_count; i++) {
            semantic_check_statement(node->children[i], stack);
        }
        scope_pop(stack);
        return;
    }
    
    /* 声明 */
    if (label_contains(label, "Declaration")) {
        semantic_check_declaration(node, stack);
        return;
    }
    
    /* if/else */
    if (label_contains(label, "IfStatement") || label_contains(label, "IfElseStatement")) {
        if (node->child_count >= 1) semantic_check_expression(node->children[0], stack);
        if (node->child_count >= 2) semantic_check_statement(node->children[1], stack);
        if (node->child_count >= 3 && label_contains(label, "IfElseStatement")) {
            semantic_check_statement(node->children[2], stack);
        }
        return;
    }
    
    /* while */
    if (label_contains(label, "WhileStatement")) {
        if (node->child_count >= 1) semantic_check_expression(node->children[0], stack);
        if (node->child_count >= 2) semantic_check_statement(node->children[1], stack);
        return;
    }
    
    /* for */
    if (label_contains(label, "ForStatement")) {
        scope_push(stack);
        if (node->child_count >= 4) {
            semantic_check_statement(node->children[0], stack);
            semantic_check_statement(node->children[1], stack);
            semantic_check_expression(node->children[2], stack);
            semantic_check_statement(node->children[3], stack);
        }
        scope_pop(stack);
        return;
    }
    
    /* return */
    if (label_contains(label, "ReturnStatement")) {
        if (node->child_count >= 1) semantic_check_expression(node->children[0], stack);
        return;
    }
    
    /* 表达式语句 */
    if (label_contains(label, "ExpressionStatement")) {
        for (i = 0; i < node->child_count; i++) {
            semantic_check_expression(node->children[i], stack);
        }
        return;
    }
    
    /* 语句列表 */
    if (label_contains(label, "StatementList")) {
        for (i = 0; i < node->child_count; i++) {
            semantic_check_statement(node->children[i], stack);
        }
        return;
    }
    
    /* 递归 */
    for (i = 0; i < node->child_count; i++) {
        semantic_check_statement(node->children[i], stack);
    }
}

/* ========== 主函数 ========== */

void semantic_analysis(ASTNode *node, ScopeStack *stack) {
    int i;
    if (node == NULL) return;
    
    printf("\n========== Starting Semantic Analysis ==========\n");
    semantic_error_count = 0;
    semantic_warning_count = 0;
    
    symbol_table_init(stack);
    
    if (label_contains(get_node_type(node), "Program")) {
        for (i = 0; i < node->child_count; i++) {
            semantic_check_statement(node->children[i], stack);
        }
    } else if (label_contains(get_node_type(node), "TranslationUnit")) {
        for (i = 0; i < node->child_count; i++) {
            semantic_check_statement(node->children[i], stack);
        }
    } else {
        semantic_check_statement(node, stack);
    }
    
    symbol_table_print(stack);
    printf("========== Semantic Analysis Complete ==========\n");
    printf("Errors: %d, Warnings: %d\n\n", semantic_error_count, semantic_warning_count);
}
