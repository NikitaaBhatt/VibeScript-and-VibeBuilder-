%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "vibe.h"
#include "interpreter.h"

void yyerror(const char *s);
int yylex(void);

/* Root of AST */
ast_node* root;
FunctionInfo* current_function;

ast_node* create_node(char* type, ast_node* left, ast_node* right, char* value) {
    ast_node* new_node = (ast_node*)malloc(sizeof(ast_node));
    new_node->node_type = strdup(type);
    new_node->left = left;
    new_node->right = right;
    new_node->type = NULL;
    if (value)
        new_node->value = strdup(value);
    else
        new_node->value = NULL;
    return new_node;
}

void print_ast(ast_node* node, int level) {
    if (!node) return;
    for (int i = 0; i < level; i++) printf("  ");
    printf("%s", node->node_type);
    if (node->value) printf(" (%s)", node->value);
    if (node->type) printf(" : %s", node->type);
    printf("\n");
    print_ast(node->left, level + 1);
    print_ast(node->right, level + 1);
}

void check_main_defined() {
    int main_found = 0;
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].name, "main") == 0 && functions[i].defined) {
            main_found = 1;
            break;
        }
    }
    if (!main_found) {
        fprintf(stderr, "Error: No main function defined\n");
        exit(1);
    }
}
%}



%union {
    int ival;
    float fval;
    char *sval;
    int boolval;
    struct Symbol *sym;
    struct ast_node *ast;
}

%token VIBE PLOT SPILL DROP YAH MAYBE NAH
%token RUNTHRU ONREPEAT DOSTART DOEND GRAB SCOOP DIP SLIDE
%token VOID_TYPE INT_TYPE FLOAT_TYPE STRING_TYPE BOOL_TYPE
%token INT FLOAT STRING BOOLVAL IDENT
%token EQ NEQ LE GE ASSIGN LT GT PLUS MINUS MUL DIV AND OR NOT
%token LPAREN RPAREN LBRACE RBRACE COMMA COLON SEMICOLON

%nonassoc UMINUS
%left OR
%left AND
%left EQ NEQ
%left LT GT LE GE
%left PLUS MINUS
%left MUL DIV
%right NOT
%right ASSIGN

%start program

%type <ast> program functions function params param_list param block statements statement declaration assignment expression conditional maybe_clauses loop dowhile func_call args arg_list return_stmt print_stmt
%type <sval> IDENT STRING
%type <ast> type 
%type <ival> INT BOOLVAL
%type <fval> FLOAT

%%

program: functions
    {
        root = $1;
        printf("=== AST ===\n");
        print_ast(root, 0);
        check_main_defined();
    }
;

functions: functions function
    { $$ = create_node("functions", $1, $2, NULL); }
    | function
    { $$ = $1; }
;

function: PLOT type IDENT LPAREN params RPAREN block
    {
        add_function($3, $2->value);
        current_function = &functions[func_count-1];
        enter_scope();
        $$ = create_node("function", $7, NULL, $3);
        $$->type = strdup($2->value);
        exit_scope();
        if (strcmp($3, "main") == 0) {
            current_function->defined = 1;
        }
        current_function = NULL;
    }
;

type: VOID_TYPE { $$ = create_node("type", NULL, NULL, "void"); }
    | INT_TYPE { $$ = create_node("type", NULL, NULL, "int"); }
    | FLOAT_TYPE { $$ = create_node("type", NULL, NULL, "float"); }
    | STRING_TYPE { $$ = create_node("type", NULL, NULL, "string"); }
    | BOOL_TYPE { $$ = create_node("type", NULL, NULL, "bool"); }
;

params: param_list
    | /* empty */ { $$ = NULL; }
;

param_list: param_list COMMA param
    { $$ = create_node("param_list", $1, $3, NULL); }
    | param
    { $$ = $1; }
;

param: type IDENT
    {
        insert_symbol($2, "param", $1->value);  
        add_function_param(current_function->name, $1->value);  // Use $1->value
    }
;

block: LBRACE 
    { enter_scope(); } 
    statements 
    RBRACE 
    { $$ = $3; exit_scope(); }
;


statements: statements statement
    { $$ = create_node("statements", $1, $2, NULL); }
    | statement
    { $$ = $1; }
;

statement:
      declaration SEMICOLON
    | assignment SEMICOLON
    | conditional
    | loop
    | dowhile
    | func_call SEMICOLON
    | return_stmt SEMICOLON
    | print_stmt SEMICOLON
    | block
;

declaration:
    VIBE type IDENT
    {
        if (lookup_current_scope($3)) {
            fprintf(stderr, "Error: Redeclaration of '%s'\n", $3);
            exit(1);
        }
        insert_symbol($3, "variable", $2->value);  // Use $2->value
        $$ = create_node("declaration", NULL, NULL, $3);
        $$->type = strdup($2->value);  // Use $2->value
    }
    | VIBE type IDENT ASSIGN expression
    {
        if (lookup_current_scope($3)) {
            fprintf(stderr, "Error: Redeclaration of '%s'\n", $3);
            exit(1);
        }
        if (strcmp($2->value, $5->type) != 0) {  // Use $2->value
            fprintf(stderr, "Error: Type mismatch in initialization of '%s'\n", $3);
            exit(1);
        }
        insert_symbol($3, "variable", $2->value);  // Use $2->value
        $$ = create_node("decl_assign", create_node("ID", NULL, NULL, $3), $5, NULL);
        $$->type = strdup($2->value);  // Use $2->value
    }
;
assignment: IDENT ASSIGN expression
    {
        Symbol* s = lookup($1);
        if (!s) {
            fprintf(stderr, "Error: Variable '%s' not declared\n", $1);
            exit(1);
        }
        if (strcmp(s->type, $3->type) != 0) {
            fprintf(stderr, "Error: Type mismatch in assignment to '%s'\n", $1);
            exit(1);
        }
        $$ = create_node("assign", create_node("ID", NULL, NULL, $1), $3, NULL);
        $$->type = strdup(s->type);
    }
;

expression:
      expression PLUS expression 
    { 
        if (strcmp($1->type, $3->type) != 0) { 
            fprintf(stderr, "Error: Type mismatch in addition\n"); 
            exit(1); 
        } 
        $$ = create_node("+", $1, $3, NULL);
        $$->type = strdup($1->type);
    }
    | expression MINUS expression 
        { 
            if (strcmp($1->type, $3->type) != 0) { 
                fprintf(stderr, "Error: Type mismatch in subtraction\n"); 
                exit(1); 
            }
            $$ = create_node("-", $1, $3, NULL);
            $$->type = $1->type;
        }
    | expression MUL expression 
        { 
            if (strcmp($1->type, $3->type) != 0) { 
                fprintf(stderr, "Error: Type mismatch in multiplication\n"); 
                exit(1); 
            } 
            $$ = create_node("*", $1, $3, NULL);
            $$->type = $1->type;
        }
    | expression DIV expression 
        { 
            if (strcmp($1->type, $3->type) != 0) { 
                fprintf(stderr, "Error: Type mismatch in division\n"); 
                exit(1); 
            } 
            $$ = create_node("/", $1, $3, NULL);
            $$->type = $1->type;
        }
    | expression EQ expression 
        { 
            if (strcmp($1->type, $3->type) != 0) { 
                fprintf(stderr, "Error: Type mismatch in equality comparison\n"); 
                exit(1); 
            }
            $$ = create_node("==", $1, $3, NULL);
            $$->type = "bool";  // Equality returns boolean
        }
    | expression NEQ expression 
        { 
            if (strcmp($1->type, $3->type) != 0) { 
                fprintf(stderr, "Error: Type mismatch in inequality comparison\n"); 
                exit(1); 
            }
            $$ = create_node("!=", $1, $3, NULL);
            $$->type = "bool";  // Inequality returns boolean
        }
    | expression LT expression 
        { 
            if (strcmp($1->type, $3->type) != 0) { 
                fprintf(stderr, "Error: Type mismatch in less-than comparison\n"); 
                exit(1); 
            }
            $$ = create_node("<", $1, $3, NULL);
            $$->type = "bool";  // Less than returns boolean
        }
    | expression GT expression 
        { 
            if (strcmp($1->type, $3->type) != 0) { 
                fprintf(stderr, "Error: Type mismatch in greater-than comparison\n"); 
                exit(1); 
            }
            $$ = create_node(">", $1, $3, NULL);
            $$->type = "bool";  // Greater than returns boolean
        }
    | expression LE expression 
        { 
            if (strcmp($1->type, $3->type) != 0) { 
                fprintf(stderr, "Error: Type mismatch in less-than-or-equal comparison\n"); 
                exit(1); 
            }
            $$ = create_node("<=", $1, $3, NULL);
            $$->type = "bool";  // Less than or equal returns boolean
        }
    | expression GE expression 
        { 
            if (strcmp($1->type, $3->type) != 0) { 
                fprintf(stderr, "Error: Type mismatch in greater-than-or-equal comparison\n"); 
                exit(1); 
            }
            $$ = create_node(">=", $1, $3, NULL);
            $$->type = "bool";  // Greater than or equal returns boolean
        }
    | expression AND expression 
        { 
            if (strcmp($1->type, "bool") != 0 || strcmp($3->type, "bool") != 0) { 
                fprintf(stderr, "Error: Type mismatch in AND operation\n"); 
                exit(1); 
            }
            $$ = create_node("AND", $1, $3, NULL);
            $$->type = "bool";  // AND returns boolean
        }
    | expression OR expression 
        { 
            if (strcmp($1->type, "bool") != 0 || strcmp($3->type, "bool") != 0) { 
                fprintf(stderr, "Error: Type mismatch in OR operation\n"); 
                exit(1); 
            }
            $$ = create_node("OR", $1, $3, NULL);
            $$->type = "bool";  // OR returns boolean
        }
    | NOT expression 
        { 
            if (strcmp($2->type, "bool") != 0) { 
                fprintf(stderr, "Error: Type mismatch in NOT operation\n"); 
                exit(1); 
            }
            $$ = create_node("NOT", $2, NULL, NULL);
            $$->type = "bool";  // NOT returns boolean
        }
    | MINUS expression %prec UMINUS 
        { 
            $$ = create_node("UMINUS", $2, NULL, NULL); 
            $$->type = $2->type;  // Unary minus retains type of the operand
        }
    | LPAREN expression RPAREN 
        { $$ = $2; }
    | IDENT 
        {
            Symbol* s = lookup($1);  // Check if variable is declared
            if (!s) {
                fprintf(stderr, "Error: Variable '%s' not declared\n", $1);
                exit(1);
            }
            $$ = create_node("ID", NULL, NULL, $1);
            $$->type = s->type;  // Set type to the variable's type
        }
    | INT 
        { 
            char buffer[20];
            sprintf(buffer, "%d", $1);
            $$ = create_node("INT", NULL, NULL, strdup(buffer));
            $$->type = "int";  // Integer type
        }
    | FLOAT 
        { 
            char buffer[20];
            sprintf(buffer, "%f", $1);
            $$ = create_node("FLOAT", NULL, NULL, strdup(buffer));
            $$->type = "float";  // Float type
        }
    | STRING 
        { 
            $$ = create_node("STRING", NULL, NULL, $1);
            $$->type = "string";  // String type
        }
    | BOOLVAL 
        { 
            char *val = ($1) ? "true" : "false";
            $$ = create_node("BOOL", NULL, NULL, strdup(val));
            $$->type = "bool";  // Boolean type
        }
    | func_call { $$ = $1; }
;

conditional: YAH expression block maybe_clauses
    {
        if (strcmp($2->type, "bool") != 0) {
            fprintf(stderr, "Error: Condition must be boolean\n");
            exit(1);
        }
        $$ = create_node("if", $2, $3, NULL);
        if ($4) {
            $$->right = create_node("else", $4, NULL, NULL);
        }
    }
;

maybe_clauses:
      MAYBE expression block maybe_clauses
    {
        if (strcmp($2->type, "bool") != 0) {
            fprintf(stderr, "Error: Condition must be boolean\n");
            exit(1);
        }
        $$ = create_node("else_if", $2, $3, NULL);
        if ($4) {
            $$->right = $4;
        }
    }
    | NAH block
    { $$ = $2; }
    | /* empty */ { $$ = NULL; }
;

loop:
    RUNTHRU LPAREN assignment SEMICOLON expression SEMICOLON assignment RPAREN block
    {
        if (strcmp($5->type, "bool") != 0) {
            fprintf(stderr, "Error: Loop condition must be boolean\n");
            exit(1);
        }
        ast_node *init = $3;
        ast_node *cond = $5;
        ast_node *incr = $7;
        ast_node *cond_incr = create_node("cond_incr", cond, incr, NULL);
        ast_node *for_head = create_node("for", init, cond_incr, NULL);
        $$ = create_node("for_loop", for_head, $9, NULL);
    }
    | ONREPEAT expression block
    {
        if (strcmp($2->type, "bool") != 0) {
            fprintf(stderr, "Error: Loop condition must be boolean\n");
            exit(1);
        }
        $$ = create_node("while_loop", $2, $3, NULL);
    }
;

dowhile: DOSTART block DOEND expression SEMICOLON
    {
        if (strcmp($4->type, "bool") != 0) {
            fprintf(stderr, "Error: Loop condition must be boolean\n");
            exit(1);
        }
        $$ = create_node("do_while", $2, $4, NULL);
    }
;

func_call: IDENT LPAREN args RPAREN
    {
        if (check_function_args($1, $3)) {
            exit(1);
        }
        $$ = create_node("call", NULL, $3, $1);
        
        // Set return type
        for (int i = 0; i < func_count; i++) {
            if (strcmp(functions[i].name, $1) == 0) {
                $$->type = strdup(functions[i].return_type);
                break;
            }
        }
    }
;

args: arg_list
    | /* empty */ { $$ = NULL; }
;

arg_list: arg_list COMMA expression
    { 
        $$ = create_node("args", $1, $3, NULL); 
        $$->type = strdup($3->type);
    }
    | expression
    { $$ = $1; }
;

return_stmt: DROP
    {
        if (current_function && strcmp(current_function->return_type, "void") != 0) {
            fprintf(stderr, "Error: Non-void function missing return value\n");
            exit(1);
        }
        $$ = create_node("return", NULL, NULL, NULL);
    }
    | DROP expression
    {
        if (current_function && verify_return_type(current_function->name, $2->type)) {
            fprintf(stderr, "Error: Return type mismatch in function '%s'\n", current_function->name);
            exit(1);
        }
        $$ = create_node("return", $2, NULL, NULL);
    }
;

print_stmt: SPILL expression
    { 
        $$ = create_node("print", $2, NULL, NULL); 
    }
;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
    exit(1);
}

int main() {
    yyparse();
    
    // After successful parsing, interpret the AST
    printf("\n===EXECUTION===\n");
    if (root) {
        interpret(root);
    } else {
        fprintf(stderr, "Error: No AST generated\n");
        return 1;
    }
    
    return 0;
}