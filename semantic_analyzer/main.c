#include "semantic_analyzer.h"

ASTNode* ast_new(const char *label) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (node == NULL) return NULL;
    node->label = (char *)malloc(strlen(label) + 1);
    strcpy(node->label, label);
    node->child_count = 0;
    node->children = NULL;
    return node;
}

ASTNode* ast_new_token(const char *kind, const char *lexeme) {
    size_t kind_len = strlen(kind);
    size_t lexeme_len = strlen(lexeme);
    char *label = (char *)malloc(kind_len + lexeme_len + 2);
    ASTNode *node;
    memcpy(label, kind, kind_len);
    label[kind_len] = ':';
    memcpy(label + kind_len + 1, lexeme, lexeme_len + 1);
    node = ast_new(label);
    free(label);
    return node;
}

void ast_add_child(ASTNode *parent, ASTNode *child) {
    ASTNode **children;
    if (parent == NULL || child == NULL) return;
    children = (ASTNode **)realloc(parent->children, sizeof(ASTNode *) * (parent->child_count + 1));
    if (children == NULL) return;
    parent->children = children;
    parent->children[parent->child_count] = child;
    parent->child_count += 1;
}

static void ast_print_indent(int indent) {
    for (int i = 0; i < indent; i++) putchar(' ');
}

void ast_print(ASTNode *node, int indent) {
    if (node == NULL) return;
    ast_print_indent(indent);
    printf("%s\n", node->label != NULL ? node->label : "<null>");
    for (int i = 0; i < node->child_count; i++) {
        ast_print(node->children[i], indent + 2);
    }
}

void ast_free(ASTNode *node) {
    if (node == NULL) return;
    for (int i = 0; i < node->child_count; i++) {
        ast_free(node->children[i]);
    }
    free(node->children);
    free(node->label);
    free(node);
}

/* Simple code parser - converts file content to AST */

#include <ctype.h>

typedef struct {
    const char *content;
    int pos;
    int len;
} Parser;

Parser* parser_new(const char *content) {
    Parser *p = (Parser *)malloc(sizeof(Parser));
    p->content = content;
    p->pos = 0;
    p->len = strlen(content);
    return p;
}

void parser_skip_whitespace(Parser *p) {
    while (p->pos < p->len && isspace(p->content[p->pos])) {
        p->pos++;
    }
}

void parser_skip_line(Parser *p) {
    while (p->pos < p->len && p->content[p->pos] != '\n') {
        p->pos++;
    }
    if (p->pos < p->len) p->pos++;
}

int parser_is_keyword(const char *word) {
    const char *keywords[] = {"int", "float", "char", "void", "if", "else", "while", "for", "return", NULL};
    for (int i = 0; keywords[i]; i++) {
        if (strcmp(word, keywords[i]) == 0) return 1;
    }
    return 0;
}

char* parser_read_word(Parser *p) {
    char buffer[256];
    int len = 0;
    parser_skip_whitespace(p);
    while (p->pos < p->len && (isalnum(p->content[p->pos]) || p->content[p->pos] == '_')) {
        buffer[len++] = p->content[p->pos++];
    }
    buffer[len] = '\0';
    char *result = (char *)malloc(len + 1);
    strcpy(result, buffer);
    return result;
}

char* parser_read_number(Parser *p) {
    char buffer[256];
    int len = 0;
    parser_skip_whitespace(p);
    while (p->pos < p->len && isdigit(p->content[p->pos])) {
        buffer[len++] = p->content[p->pos++];
    }
    buffer[len] = '\0';
    char *result = (char *)malloc(len + 1);
    strcpy(result, buffer);
    return result;
}

ASTNode* parse_simple_code(const char *content) {
    Parser *p = parser_new(content);
    ASTNode *program = ast_new("Program");
    ASTNode *trans_unit = ast_new("TranslationUnit");
    ast_add_child(program, trans_unit);
    
    while (p->pos < p->len) {
        parser_skip_whitespace(p);
        if (p->pos >= p->len) break;
        
        char *word = parser_read_word(p);
        
        if (strlen(word) == 0) {
            free(word);
            parser_skip_line(p);
            continue;
        }
        
        if (parser_is_keyword(word)) {
            if (strcmp(word, "int") == 0 || strcmp(word, "float") == 0 || 
                strcmp(word, "char") == 0 || strcmp(word, "void") == 0) {
                /* 处理声明 */
                ASTNode *decl = ast_new("Declaration");
                ASTNode *type_spec = ast_new_token("Type", word);
                ast_add_child(decl, type_spec);
                
                ASTNode *decl_list = ast_new("InitDeclaratorList");
                char *var_name = parser_read_word(p);
                ast_add_child(decl_list, ast_new_token("Declarator", var_name));
                ast_add_child(decl, decl_list);
                ast_add_child(trans_unit, decl);
                free(var_name);
            }
            free(word);
            parser_skip_line(p);
        } else {
            /* 处理赋值或其他语句 */
            ASTNode *expr_stmt = ast_new("ExpressionStatement");
            ASTNode *assign = ast_new("Assign");
            ast_add_child(assign, ast_new_token("Identifier", word));
            
            parser_skip_whitespace(p);
            if (p->pos < p->len && p->content[p->pos] == '=') {
                p->pos++;
                parser_skip_whitespace(p);
                char *value = parser_read_word(p);
                if (strlen(value) == 0) {
                    free(value);
                    value = parser_read_number(p);
                }
                ast_add_child(assign, ast_new_token("Value", value));
                free(value);
            }
            
            ast_add_child(expr_stmt, assign);
            ast_add_child(trans_unit, expr_stmt);
            parser_skip_line(p);
        }
    }
    
    free(p);
    return program;
}

char* read_file(const char *filename, int *size) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) return NULL;
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *content = (char *)malloc(file_size + 1);
    if (content == NULL) {
        fclose(file);
        return NULL;
    }
    
    size_t read_size = fread(content, 1, file_size, file);
    content[read_size] = '\0';
    fclose(file);
    
    if (size) *size = read_size;
    return content;
}

void run_file_test(const char *filename) {
    int size;
    char *content = read_file(filename, &size);
    if (content == NULL) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return;
    }
    
    printf("\n================================================\n");
    printf("  Testing: %s\n", filename);
    printf("================================================\n");
    printf("\nSource code:\n---\n%s---\n", content);
    
    ASTNode *ast = parse_simple_code(content);
    ScopeStack stack;
    
    printf("\n========== Abstract Syntax Tree ==========\n");
    ast_print(ast, 0);
    printf("==========================================\n");
    
    semantic_analysis(ast, &stack);
    
    if (semantic_error_count == 0 && semantic_warning_count == 0) {
        printf("[OK] No errors or warnings found.\n\n");
    } else {
        printf("[ERROR] Found %d error(s) and %d warning(s).\n\n",
               semantic_error_count, semantic_warning_count);
    }
    
    symbol_table_free(&stack);
    ast_free(ast);
    free(content);
}

int main(int argc, char **argv) {
    printf("\n================================================\n");
    printf("   Semantic Analyzer - File Based Tests\n");
    printf("================================================\n\n");
    
    if (argc > 1) {
        /* Test specified files */
        for (int i = 1; i < argc; i++) {
            run_file_test(argv[i]);
        }
    } else {
        /* Display usage instructions */
        printf("Usage: %s <file1.c> <file2.c> ...\n", argv[0]);
        printf("Or place test files in tests/ folder.\n");
        printf("\nExample:\n");
        printf("  %s tests/test1.c tests/test2.c\n", argv[0]);
        printf("  %s tests/*.c\n\n", argv[0]);
        
        printf("Available test files:\n");
        printf("  tests/test1.c - Simple declaration\n");
        printf("  tests/test2.c - Nested scopes\n");
        printf("  tests/test3.c - Error detection\n");
        printf("  tests/test4.c - Type mixing\n\n");
        
        printf("To run all tests, use:\n");
        printf("  %s tests/test1.c tests/test2.c tests/test3.c tests/test4.c\n\n", argv[0]);
    }
    
    return 0;
}
