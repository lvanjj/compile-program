#include "target_code.h"

#include <stdlib.h>
#include <string.h>

int yyparse(void);
extern FILE *yyin;
extern ASTNode *syntax_root;

static void ast_free(ASTNode *node) {
    int i;

    if (node == NULL) {
        return;
    }

    for (i = 0; i < node->child_count; i++) {
        ast_free(node->children[i]);
    }

    free(node->children);
    free(node->label);
    free(node);
}

static int generate_from_source(const char *source_path, const char *output_path) {
    FILE *input_file;
    int parse_result;
    ASTNode *root;

    input_file = fopen(source_path, "r");
    if (input_file == NULL) {
        fprintf(stderr, "Cannot open source file: %s\n", source_path);
        return 1;
    }

    yyin = input_file;
    syntax_root = NULL;
    parse_result = yyparse();
    fclose(input_file);

    if (parse_result != 0 || syntax_root == NULL) {
        fprintf(stderr, "Syntax analysis failed for %s\n", source_path);
        return 1;
    }

    root = syntax_root;
    if (tc_generate_to_file(root, output_path) != 0) {
        fprintf(stderr, "Failed to write target code to %s\n", output_path);
        ast_free(root);
        syntax_root = NULL;
        return 1;
    }

    ast_free(root);
    syntax_root = NULL;
    return 0;
}

int main(int argc, char **argv) {
    const char *source_path = "..\\semantic_analyzer\\tests\\test1.c";
    const char *output_path = "target_code.asm";

    if (argc > 1) {
        source_path = argv[1];
    }
    if (argc > 2) {
        output_path = argv[2];
    }

    if (generate_from_source(source_path, output_path) != 0) {
        return 1;
    }

    printf("Target code generated from %s -> %s\n", source_path, output_path);
    return 0;
}