#ifndef TARGET_CODE_H
#define TARGET_CODE_H

#include <stdio.h>
#include "semantic_analyzer.h"

int tc_generate_to_stream(ASTNode *root, FILE *out);
int tc_generate_to_file(ASTNode *root, const char *output_path);
int tc_generate_from_source_file(const char *source_path, const char *output_path);

#endif