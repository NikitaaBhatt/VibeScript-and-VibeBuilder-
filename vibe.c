#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "vibe.h"

Symbol symtab[1000];
int symcount = 0;
int current_scope = 0;
FunctionInfo functions[100];
int func_count = 0;

void insert_symbol(const char* name, const char* role, const char* type) {
    for (int i = 0; i < symcount; i++) {
        if (strcmp(symtab[i].name, name) == 0 && symtab[i].scope_level == current_scope) {
            fprintf(stderr, "Error: Redeclaration of '%s' in same scope\n", name);
            exit(1);
        }
    }
    strcpy(symtab[symcount].name, name);
    strcpy(symtab[symcount].role, role);
    strcpy(symtab[symcount].type, type);
    symtab[symcount].scope_level = current_scope;
    symcount++;
}

Symbol* lookup(const char* name) {
    for (int i = symcount - 1; i >= 0; i--) {
        if (strcmp(symtab[i].name, name) == 0) {
            return &symtab[i];
        }
    }
    return NULL;
}

Symbol* lookup_current_scope(const char* name) {
    for (int i = symcount - 1; i >= 0; i--) {
        if (strcmp(symtab[i].name, name) == 0 && symtab[i].scope_level == current_scope) {
            return &symtab[i];
        }
    }
    return NULL;
}

void enter_scope() {
    current_scope++;
}

void exit_scope() {
    // Remove symbols from current scope
    for (int i = symcount - 1; i >= 0; i--) {
        if (symtab[i].scope_level == current_scope) {
            symcount--;
        } else {
            break;
        }
    }
    current_scope--;
}

void add_function(const char* name, const char* return_type) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            fprintf(stderr, "Error: Function '%s' already declared\n", name);
            exit(1);
        }
    }
    strcpy(functions[func_count].name, name);
    strcpy(functions[func_count].return_type, return_type);
    functions[func_count].param_count = 0;
    functions[func_count].defined = 0;
    func_count++;
}

void add_function_param(const char* func_name, const char* param_type) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].name, func_name) == 0) {
            if (functions[i].param_count >= 10) {
                fprintf(stderr, "Error: Too many parameters for function '%s'\n", func_name);
                exit(1);
            }
            strcpy(functions[i].param_types[functions[i].param_count], param_type);
            functions[i].param_count++;
            return;
        }
    }
    fprintf(stderr, "Error: Function '%s' not found when adding param\n", func_name);
    exit(1);
}

int check_function_args(const char* func_name, ast_node* args) {
    FunctionInfo* func = NULL;
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].name, func_name) == 0) {
            func = &functions[i];
            break;
        }
    }
    if (!func) {
        fprintf(stderr, "Error: Function '%s' not declared\n", func_name);
        return 1;
    }

    int arg_count = 0;
    ast_node* current = args;
    while (current) {
        arg_count++;
        if (current->node_type && strcmp(current->node_type, "args") == 0) {
            current = current->right;
        } else {
            current = NULL;
        }
    }

    if (arg_count != func->param_count) {
        fprintf(stderr, "Error: Argument count mismatch for function '%s'\n", func_name);
        return 1;
    }

    current = args;
    for (int i = 0; i < func->param_count; i++) {
        if (strcmp(current->type, func->param_types[i]) != 0) {
            fprintf(stderr, "Error: Argument type mismatch for function '%s' (param %d)\n", func_name, i+1);
            return 1;
        }
        if (current->node_type && strcmp(current->node_type, "args") == 0) {
            current = current->left;
        }
    }
    return 0;
}

int verify_return_type(const char* func_name, const char* return_type) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].name, func_name) == 0) {
            if (strcmp(functions[i].return_type, return_type) != 0) {
                fprintf(stderr, "Error: Return type mismatch for function '%s'\n", func_name);
                return 1;
            }
            return 0;
        }
    }
    fprintf(stderr, "Error: Function '%s' not found\n", func_name);
    return 1;
}