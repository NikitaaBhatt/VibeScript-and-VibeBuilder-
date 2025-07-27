#ifndef AST_H
#define AST_H

typedef struct ast_node {
    char* node_type;      // e.g., "args", "identifier", etc.
    char* type;           // e.g., "int", "float", etc.
    char* value;          // node value
    struct ast_node* left;
    struct ast_node* right;
} ast_node;

#endif