#include "target_code.h"

#include <stdarg.h>

typedef struct CodeGenerator {
    FILE *out;
    int temp_index;
    int label_index;
    char *break_labels[128];
    char *continue_labels[128];
    int loop_depth;
} CodeGenerator;

static char *tc_strdup(const char *text) {
    size_t length;
    char *copy;

    if (text == NULL) {
        return NULL;
    }

    length = strlen(text) + 1;
    copy = (char *)malloc(length);
    if (copy != NULL) {
        memcpy(copy, text, length);
    }
    return copy;
}

static const char *node_label(ASTNode *node) {
    if (node == NULL || node->label == NULL) {
        return "";
    }
    return node->label;
}

static int label_equals(ASTNode *node, const char *text) {
    return strcmp(node_label(node), text) == 0;
}

static int label_contains(ASTNode *node, const char *text) {
    return strstr(node_label(node), text) != NULL;
}

static const char *label_after_colon(ASTNode *node) {
    const char *label = node_label(node);
    const char *colon = strchr(label, ':');
    if (colon != NULL && *(colon + 1) != '\0') {
        return colon + 1;
    }
    return label;
}

static void emitf(CodeGenerator *gen, const char *fmt, ...) {
    va_list args;

    if (gen == NULL || gen->out == NULL || fmt == NULL) {
        return;
    }

    va_start(args, fmt);
    vfprintf(gen->out, fmt, args);
    va_end(args);
    fputc('\n', gen->out);
}

static char *new_temp(CodeGenerator *gen) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "t%d", gen->temp_index++);
    return tc_strdup(buffer);
}

static char *new_label(CodeGenerator *gen, const char *prefix) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s%d", prefix, gen->label_index++);
    return tc_strdup(buffer);
}

static void push_loop(CodeGenerator *gen, char *break_label, char *continue_label) {
    if (gen->loop_depth >= 128) {
        return;
    }
    gen->break_labels[gen->loop_depth] = break_label;
    gen->continue_labels[gen->loop_depth] = continue_label;
    gen->loop_depth++;
}

static void pop_loop(CodeGenerator *gen) {
    if (gen->loop_depth <= 0) {
        return;
    }
    gen->loop_depth--;
}

static const char *current_break_label(CodeGenerator *gen) {
    if (gen->loop_depth <= 0) {
        return NULL;
    }
    return gen->break_labels[gen->loop_depth - 1];
}

static const char *current_continue_label(CodeGenerator *gen) {
    if (gen->loop_depth <= 0) {
        return NULL;
    }
    return gen->continue_labels[gen->loop_depth - 1];
}

static void free_loop_labels(CodeGenerator *gen) {
    int i;

    for (i = 0; i < gen->loop_depth; i++) {
        free(gen->break_labels[i]);
        free(gen->continue_labels[i]);
        gen->break_labels[i] = NULL;
        gen->continue_labels[i] = NULL;
    }
}

static char *generate_expression(CodeGenerator *gen, ASTNode *node);
static void generate_statement(CodeGenerator *gen, ASTNode *node);

static char *generate_binary(CodeGenerator *gen, ASTNode *node, const char *op_name, const char *cmp_name, const char *set_name) {
    char *left;
    char *right;
    char *result;

    if (node == NULL || node->child_count < 2) {
        return tc_strdup("0");
    }

    left = generate_expression(gen, node->children[0]);
    right = generate_expression(gen, node->children[1]);
    result = new_temp(gen);

    if (op_name != NULL) {
        emitf(gen, "MOV %s, %s", result, left);
        emitf(gen, "%s %s, %s", op_name, result, right);
    } else {
        emitf(gen, "%s %s, %s", cmp_name, left, right);
        emitf(gen, "%s %s", set_name, result);
    }

    free(left);
    free(right);
    return result;
}

static char *generate_expression(CodeGenerator *gen, ASTNode *node) {
    const char *label;
    char *left;
    char *right;
    char *result;

    if (node == NULL) {
        return tc_strdup("0");
    }

    label = node_label(node);

    if (strncmp(label, "Identifier:", 11) == 0) {
        return tc_strdup(label_after_colon(node));
    }

    if (strncmp(label, "IntConst:", 9) == 0 ||
        strncmp(label, "FloatConst:", 11) == 0 ||
        strncmp(label, "CharLiteral:", 12) == 0 ||
        strncmp(label, "StringLiteral:", 14) == 0) {
        return tc_strdup(label_after_colon(node));
    }

    if (label_equals(node, "Assign")) {
        if (node->child_count < 2) {
            return tc_strdup("0");
        }
        left = generate_expression(gen, node->children[0]);
        right = generate_expression(gen, node->children[1]);
        emitf(gen, "MOV %s, %s", left, right);
        free(right);
        return left;
    }

    if (label_equals(node, "Add")) {
        return generate_binary(gen, node, "ADD", NULL, NULL);
    }
    if (label_equals(node, "Sub")) {
        return generate_binary(gen, node, "SUB", NULL, NULL);
    }
    if (label_equals(node, "Mul")) {
        return generate_binary(gen, node, "MUL", NULL, NULL);
    }
    if (label_equals(node, "Div")) {
        return generate_binary(gen, node, "DIV", NULL, NULL);
    }
    if (label_equals(node, "Mod")) {
        return generate_binary(gen, node, "MOD", NULL, NULL);
    }
    if (label_equals(node, "LogicalAnd")) {
        return generate_binary(gen, node, "AND", NULL, NULL);
    }
    if (label_equals(node, "LogicalOr")) {
        return generate_binary(gen, node, "OR", NULL, NULL);
    }
    if (label_equals(node, "Negate")) {
        if (node->child_count < 1) {
            return tc_strdup("0");
        }
        left = generate_expression(gen, node->children[0]);
        result = new_temp(gen);
        emitf(gen, "MOV %s, %s", result, left);
        emitf(gen, "NEG %s", result);
        free(left);
        return result;
    }
    if (label_equals(node, "Not")) {
        if (node->child_count < 1) {
            return tc_strdup("0");
        }
        left = generate_expression(gen, node->children[0]);
        result = new_temp(gen);
        emitf(gen, "MOV %s, %s", result, left);
        emitf(gen, "NOT %s", result);
        free(left);
        return result;
    }

    if (label_equals(node, "Equal")) {
        return generate_binary(gen, node, NULL, "CMP", "SETE");
    }
    if (label_equals(node, "NotEqual")) {
        return generate_binary(gen, node, NULL, "CMP", "SETNE");
    }
    if (label_equals(node, "LessThan")) {
        return generate_binary(gen, node, NULL, "CMP", "SETLT");
    }
    if (label_equals(node, "LessEqual")) {
        return generate_binary(gen, node, NULL, "CMP", "SETLE");
    }
    if (label_equals(node, "GreaterThan")) {
        return generate_binary(gen, node, NULL, "CMP", "SETGT");
    }
    if (label_equals(node, "GreaterEqual")) {
        return generate_binary(gen, node, NULL, "CMP", "SETGE");
    }

    if (node->child_count == 1) {
        return generate_expression(gen, node->children[0]);
    }

    if (node->child_count > 0) {
        return generate_expression(gen, node->children[node->child_count - 1]);
    }

    return tc_strdup("0");
}

static void generate_declaration(CodeGenerator *gen, ASTNode *node) {
    const char *type_name;
    ASTNode *list_node;
    int i;

    if (node == NULL || node->child_count < 2) {
        return;
    }

    type_name = label_after_colon(node->children[0]);
    list_node = node->children[1];
    emitf(gen, "; declaration %s", type_name);

    if (list_node == NULL) {
        return;
    }

    for (i = 0; i < list_node->child_count; i++) {
        ASTNode *item = list_node->children[i];

        if (item == NULL) {
            continue;
        }

        if (label_contains(item, "Declarator:")) {
            emitf(gen, "DECL %s %s", type_name, label_after_colon(item));
        } else if (label_equals(item, "InitDeclarator") && item->child_count >= 2) {
            char *value = generate_expression(gen, item->children[1]);
            const char *name = label_after_colon(item->children[0]);

            emitf(gen, "DECL %s %s", type_name, name);
            emitf(gen, "MOV %s, %s", name, value);
            free(value);
        }
    }
}

static void generate_if_statement(CodeGenerator *gen, ASTNode *node) {
    char *false_label;
    char *end_label;
    char *cond;

    if (node == NULL || node->child_count < 2) {
        return;
    }

    false_label = new_label(gen, "L_else_");
    end_label = new_label(gen, "L_end_");
    cond = generate_expression(gen, node->children[0]);
    emitf(gen, "CMP %s, 0", cond);
    emitf(gen, "JE %s", false_label);
    free(cond);

    generate_statement(gen, node->children[1]);

    if (label_equals(node, "IfElseStatement") && node->child_count >= 3) {
        emitf(gen, "JMP %s", end_label);
        emitf(gen, "%s:", false_label);
        generate_statement(gen, node->children[2]);
        emitf(gen, "%s:", end_label);
    } else {
        emitf(gen, "%s:", false_label);
    }

    free(false_label);
    free(end_label);
}

static void generate_while_statement(CodeGenerator *gen, ASTNode *node) {
    char *start_label;
    char *end_label;
    char *cond;

    if (node == NULL || node->child_count < 2) {
        return;
    }

    start_label = new_label(gen, "L_while_");
    end_label = new_label(gen, "L_end_");
    push_loop(gen, tc_strdup(end_label), tc_strdup(start_label));

    emitf(gen, "%s:", start_label);
    cond = generate_expression(gen, node->children[0]);
    emitf(gen, "CMP %s, 0", cond);
    emitf(gen, "JE %s", end_label);
    free(cond);

    generate_statement(gen, node->children[1]);
    emitf(gen, "JMP %s", start_label);
    emitf(gen, "%s:", end_label);

    pop_loop(gen);
    free(start_label);
    free(end_label);
}

static void generate_for_statement(CodeGenerator *gen, ASTNode *node) {
    char *start_label;
    char *step_label;
    char *end_label;
    char *cond;

    if (node == NULL || node->child_count < 4) {
        return;
    }

    start_label = new_label(gen, "L_for_");
    step_label = new_label(gen, "L_step_");
    end_label = new_label(gen, "L_end_");
    push_loop(gen, tc_strdup(end_label), tc_strdup(step_label));

    if (node->children[0] != NULL && node->children[0]->child_count > 0) {
        generate_statement(gen, node->children[0]);
    }

    emitf(gen, "%s:", start_label);

    if (node->children[1] != NULL && node->children[1]->child_count > 0) {
        cond = generate_expression(gen, node->children[1]->children[0]);
        emitf(gen, "CMP %s, 0", cond);
        emitf(gen, "JE %s", end_label);
        free(cond);
    }

    generate_statement(gen, node->children[3]);

    emitf(gen, "%s:", step_label);
    if (node->children[2] != NULL && !label_contains(node->children[2], "<empty>")) {
        cond = generate_expression(gen, node->children[2]);
        free(cond);
    }
    emitf(gen, "JMP %s", start_label);
    emitf(gen, "%s:", end_label);

    pop_loop(gen);
    free(start_label);
    free(step_label);
    free(end_label);
}

static void generate_statement(CodeGenerator *gen, ASTNode *node) {
    int i;

    if (node == NULL) {
        return;
    }

    if (label_contains(node, "<error>")) {
        emitf(gen, "; skipped error node: %s", node_label(node));
        return;
    }

    if (label_equals(node, "Program") ||
        label_equals(node, "TranslationUnit") ||
        label_equals(node, "StatementList") ||
        label_equals(node, "StatementList<empty>") ||
        label_equals(node, "CompoundStatement")) {
        for (i = 0; i < node->child_count; i++) {
            generate_statement(gen, node->children[i]);
        }
        return;
    }

    if (label_equals(node, "Declaration")) {
        generate_declaration(gen, node);
        return;
    }

    if (label_equals(node, "ExpressionStatement")) {
        if (node->child_count > 0) {
            char *value = generate_expression(gen, node->children[0]);
            free(value);
        }
        return;
    }

    if (label_equals(node, "EmptyExpressionStatement")) {
        return;
    }

    if (label_equals(node, "IfStatement") || label_equals(node, "IfElseStatement")) {
        generate_if_statement(gen, node);
        return;
    }

    if (label_equals(node, "WhileStatement")) {
        generate_while_statement(gen, node);
        return;
    }

    if (label_equals(node, "ForStatement")) {
        generate_for_statement(gen, node);
        return;
    }

    if (label_equals(node, "ReturnStatement")) {
        if (node->child_count > 0 && !label_contains(node->children[0], "<empty>")) {
            char *value = generate_expression(gen, node->children[0]);
            emitf(gen, "RET %s", value);
            free(value);
        } else {
            emitf(gen, "RET");
        }
        return;
    }

    if (label_equals(node, "BreakStatement")) {
        const char *target = current_break_label(gen);
        if (target != NULL) {
            emitf(gen, "JMP %s", target);
        } else {
            emitf(gen, "; break outside loop");
        }
        return;
    }

    if (label_equals(node, "ContinueStatement")) {
        const char *target = current_continue_label(gen);
        if (target != NULL) {
            emitf(gen, "JMP %s", target);
        } else {
            emitf(gen, "; continue outside loop");
        }
        return;
    }

    if (node->child_count > 0) {
        for (i = 0; i < node->child_count; i++) {
            generate_statement(gen, node->children[i]);
        }
        return;
    }

    if (strlen(node_label(node)) > 0) {
        emitf(gen, "; unhandled node: %s", node_label(node));
    }
}

int tc_generate_to_stream(ASTNode *root, FILE *out) {
    CodeGenerator gen;

    if (out == NULL) {
        return -1;
    }

    memset(&gen, 0, sizeof(gen));
    gen.out = out;

    emitf(&gen, "; target code generated from AST");
    emitf(&gen, "; output is pseudo assembly and can be adapted to x86 or MIPS");
    emitf(&gen, "BEGIN_PROGRAM");

    generate_statement(&gen, root);

    emitf(&gen, "END_PROGRAM");
    free_loop_labels(&gen);
    return 0;
}

int tc_generate_to_file(ASTNode *root, const char *output_path) {
    FILE *out;
    int result;

    if (output_path == NULL) {
        return -1;
    }

    out = fopen(output_path, "w");
    if (out == NULL) {
        return -1;
    }

    result = tc_generate_to_stream(root, out);
    fclose(out);
    return result;
}