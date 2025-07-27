#ifndef VIBE_H
#define VIBE_H

typedef struct Symbol {
    char name[100];
    char type[20];
    char role[20];
    int scope_level;
} Symbol;

typedef struct FunctionInfo {
    char name[100];
    char return_type[20];
    char param_types[10][20];
    int param_count;
    int defined;
} FunctionInfo;

extern Symbol symtab[1000];
extern int symcount;
extern int current_scope;
extern FunctionInfo functions[100];
extern int func_count;

void insert_symbol(const char* name, const char* role, const char* type);
Symbol* lookup(const char* name);
Symbol* lookup_current_scope(const char* name);
void enter_scope();
void exit_scope();

struct ast_node;
typedef struct ast_node ast_node;

int check_function_args(const char* func_name, ast_node* args);
void add_function(const char* name, const char* return_type);
void add_function_param(const char* func_name, const char* param_type);
int verify_return_type(const char* func_name, const char* return_type);

void interpret(ast_node* node);
void interpret_program();

#endif