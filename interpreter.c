#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"
#include "vibe.h"

extern ast_node* root;

typedef struct {
    char* name;
    char* type;
    union {
        int int_val;
        float float_val;
        char* string_val;
        bool bool_val;
    } value;
} Variable;

Variable variables[1000];
int var_count = 0;



// Function prototypes
void interpret(ast_node* node);
void interpret_function(ast_node* node);
void interpret_block(ast_node* node);
void interpret_declaration(ast_node* node);
void interpret_assignment(ast_node* node);
void interpret_expression(ast_node* node);
void interpret_conditional(ast_node* node);
void interpret_loop(ast_node* node);
void interpret_dowhile(ast_node* node);
void interpret_func_call(ast_node* node);
void interpret_return(ast_node* node);
void interpret_print(ast_node* node);

Variable evaluate_expression(ast_node* node);
Variable evaluate_binary_op(ast_node* node);
Variable evaluate_unary_op(ast_node* node);
Variable evaluate_identifier(ast_node* node);
Variable evaluate_literal(ast_node* node);

Variable create_variable(const char* type, const char* value_str);
Variable* find_variable(const char* name);
void print_variable(Variable var);

// Main interpreter function
void interpret(ast_node* node) {
    if (!node) return;

    if (strcmp(node->node_type, "functions") == 0) {
        interpret(node->left);
        interpret(node->right);
    } 
    else if (strcmp(node->node_type, "function") == 0) {
        interpret_function(node);
    }
    else if (strcmp(node->node_type, "block") == 0) {
        interpret_block(node);
    }
    else if (strcmp(node->node_type, "decl_assign") == 0) {
        Variable value = evaluate_expression(node->right);
        variables[var_count].name = strdup(node->left->value);
        variables[var_count].type = strdup(node->type);
        variables[var_count].value = value.value;
        var_count++;
    }
    else if (strcmp(node->node_type, "assign") == 0) {
        interpret_assignment(node);
    }
    else if (strcmp(node->node_type, "if") == 0 || 
             strcmp(node->node_type, "else_if") == 0 ||
             strcmp(node->node_type, "else") == 0) {
        interpret_conditional(node);
    }
    else if (strcmp(node->node_type, "for_loop") == 0 || 
             strcmp(node->node_type, "while_loop") == 0) {
        interpret_loop(node);
    }
    else if (strcmp(node->node_type, "do_while") == 0) {
        interpret_dowhile(node);
    }
    else if (strcmp(node->node_type, "call") == 0) {
        interpret_func_call(node);
    }
    else if (strcmp(node->node_type, "return") == 0) {
        interpret_return(node);
    }
    else if (strcmp(node->node_type, "print") == 0) {
        Variable value = evaluate_expression(node->left);
        print_variable(value);
    }
    else if (strcmp(node->node_type, "statements") == 0) {
        interpret(node->left);
        interpret(node->right);
    }
    else {
        fprintf(stderr, "Unknown node type: %s\n", node->node_type);
        exit(1);
    }
}

void interpret_function(ast_node* node) {
    if (strcmp(node->value, "main") == 0) {
        interpret(node->left);
    }
}

void interpret_block(ast_node* node) {
    enter_scope();
    interpret(node->left);
    exit_scope();
}

void interpret_assignment(ast_node* node) {
    Variable* var = find_variable(node->left->value);
    if (!var) {
        fprintf(stderr, "Error: Variable '%s' not found\n", node->left->value);
        exit(1);
    }

    Variable value = evaluate_expression(node->right);
    if (strcmp(var->type, value.type) != 0) {
        fprintf(stderr, "Error: Type mismatch in assignment to '%s'\n", node->left->value);
        exit(1);
    }

    var->value = value.value;
}

void interpret_conditional(ast_node* node) {
    if (strcmp(node->node_type, "if") == 0) {
        Variable cond = evaluate_expression(node->left);
        if (strcmp(cond.type, "bool") != 0) {
            fprintf(stderr, "Error: Condition must be boolean\n");
            exit(1);
        }
        if (cond.value.bool_val) {
            interpret(node->right);
        } else if (node->right && node->right->right) {
            interpret(node->right->right);
        }
    } 
    else if (strcmp(node->node_type, "else_if") == 0) {
        Variable cond = evaluate_expression(node->left);
        if (strcmp(cond.type, "bool") != 0) {
            fprintf(stderr, "Error: Condition must be boolean\n");
            exit(1);
        }
        if (cond.value.bool_val) {
            interpret(node->right);
        } else if (node->right) {
            interpret(node->right);
        }
    }
    else {
        interpret(node->left);
    }
}

void interpret_loop(ast_node* node) {
    if (strcmp(node->node_type, "for_loop") == 0) {
        interpret(node->left->left);
        while (1) {
            Variable cond = evaluate_expression(node->left->right->left);
            if (strcmp(cond.type, "bool") != 0) {
                fprintf(stderr, "Error: Loop condition must be boolean\n");
                exit(1);
            }
            if (!cond.value.bool_val) break;
            interpret(node->right);
            interpret(node->left->right->right);
        }
    } 
    else {
        while (1) {
            Variable cond = evaluate_expression(node->left);
            if (strcmp(cond.type, "bool") != 0) {
                fprintf(stderr, "Error: Loop condition must be boolean\n");
                exit(1);
            }
            if (!cond.value.bool_val) break;
            interpret(node->right);
        }
    }
}

void interpret_dowhile(ast_node* node) {
    Variable cond;
    do {
        interpret(node->left);
        cond = evaluate_expression(node->right);
        if (strcmp(cond.type, "bool") != 0) {
            fprintf(stderr, "Error: Loop condition must be boolean\n");
            exit(1);
        }
    } while (cond.value.bool_val);
}

void interpret_func_call(ast_node* node) {
    if (strcmp(node->value, "print") == 0) {
        interpret_print(node);
        return;
    }
    ast_node* args = node->right;
    while (args) {
        evaluate_expression(args);
        if (args->node_type && strcmp(args->node_type, "args") == 0) {
            args = args->left;
        } else {
            args = NULL;
        }
    }
}

void interpret_return(ast_node* node) {
    if (node->left) {
        evaluate_expression(node->left);
    }
    exit(0);
}

void interpret_print(ast_node* node) {
    if (strcmp(node->node_type, "print") == 0) {
        Variable value = evaluate_expression(node->left);
        print_variable(value);
    } 
    else {
        ast_node* args = node->right;
        while (args) {
            Variable value = evaluate_expression(args);
            print_variable(value);
            if (args->node_type && strcmp(args->node_type, "args") == 0) {
                args = args->left;
            } else {
                args = NULL;
            }
        }
    }
}
Variable evaluate_binary_op(ast_node* node) {
    Variable left = evaluate_expression(node->left);
    Variable right = evaluate_expression(node->right);
    Variable result;

    if (strcmp(node->node_type, "+") == 0) {
        if (strcmp(left.type, "int") == 0 && strcmp(right.type, "int") == 0) {
            result.type = strdup("int");
            result.value.int_val = left.value.int_val + right.value.int_val;
        } else if (strcmp(left.type, "float") == 0 && strcmp(right.type, "float") == 0) {
            result.type = strdup("float");
            result.value.float_val = left.value.float_val + right.value.float_val;
        } else if (strcmp(left.type, "string") == 0 && strcmp(right.type, "string") == 0) {
            result.type = strdup("string");
            char* new_str = malloc(strlen(left.value.string_val) + strlen(right.value.string_val) + 1);
            strcpy(new_str, left.value.string_val);
            strcat(new_str, right.value.string_val);
            result.value.string_val = new_str;
        } else {
            fprintf(stderr, "Error: Invalid operands for +\n");
            exit(1);
        }
    }
    else if (strcmp(node->node_type, "-") == 0) {
        if (strcmp(left.type, "int") == 0 && strcmp(right.type, "int") == 0) {
            result.type = strdup("int");
            result.value.int_val = left.value.int_val - right.value.int_val;
        } else if (strcmp(left.type, "float") == 0 && strcmp(right.type, "float") == 0) {
            result.type = strdup("float");
            result.value.float_val = left.value.float_val - right.value.float_val;
        } else {
            fprintf(stderr, "Error: Invalid operands for -\n");
            exit(1);
        }
    }
    else if (strcmp(node->node_type, "*") == 0) {
        if (strcmp(left.type, "int") == 0 && strcmp(right.type, "int") == 0) {
            result.type = strdup("int");
            result.value.int_val = left.value.int_val * right.value.int_val;
        } else if (strcmp(left.type, "float") == 0 && strcmp(right.type, "float") == 0) {
            result.type = strdup("float");
            result.value.float_val = left.value.float_val * right.value.float_val;
        } else {
            fprintf(stderr, "Error: Invalid operands for *\n");
            exit(1);
        }
    }
    else if (strcmp(node->node_type, "/") == 0) {
        if (strcmp(left.type, "int") == 0 && strcmp(right.type, "int") == 0) {
            if (right.value.int_val == 0) {
                fprintf(stderr, "Error: Division by zero\n");
                exit(1);
            }
            result.type = strdup("int");
            result.value.int_val = left.value.int_val / right.value.int_val;
        } else if (strcmp(left.type, "float") == 0 && strcmp(right.type, "float") == 0) {
            if (right.value.float_val == 0.0) {
                fprintf(stderr, "Error: Division by zero\n");
                exit(1);
            }
            result.type = strdup("float");
            result.value.float_val = left.value.float_val / right.value.float_val;
        } else {
            fprintf(stderr, "Error: Invalid operands for /\n");
            exit(1);
        }
    }
    else if (strcmp(node->node_type, "==") == 0 || strcmp(node->node_type, "!=") == 0) {
        if (strcmp(left.type, right.type) != 0) {
            fprintf(stderr, "Error: Type mismatch in comparison\n");
            exit(1);
        }
        result.type = strdup("bool");
        bool equal;
        if (strcmp(left.type, "int") == 0) {
            equal = left.value.int_val == right.value.int_val;
        } else if (strcmp(left.type, "float") == 0) {
            equal = left.value.float_val == right.value.float_val;
        } else if (strcmp(left.type, "string") == 0) {
            equal = strcmp(left.value.string_val, right.value.string_val) == 0;
        } else if (strcmp(left.type, "bool") == 0) {
            equal = left.value.bool_val == right.value.bool_val;
        } else {
            fprintf(stderr, "Error: Unsupported type in comparison\n");
            exit(1);
        }
        result.value.bool_val = (strcmp(node->node_type, "==") == 0) ? equal : !equal;
    }
    else if (strcmp(node->node_type, "<") == 0 || strcmp(node->node_type, ">") == 0 ||
             strcmp(node->node_type, "<=") == 0 || strcmp(node->node_type, ">=") == 0) {
        result.type = strdup("bool");
        if (strcmp(left.type, "int") == 0 && strcmp(right.type, "int") == 0) {
            if (strcmp(node->node_type, "<") == 0)
                result.value.bool_val = left.value.int_val < right.value.int_val;
            else if (strcmp(node->node_type, ">") == 0)
                result.value.bool_val = left.value.int_val > right.value.int_val;
            else if (strcmp(node->node_type, "<=") == 0)
                result.value.bool_val = left.value.int_val <= right.value.int_val;
            else
                result.value.bool_val = left.value.int_val >= right.value.int_val;
        } else if (strcmp(left.type, "float") == 0 && strcmp(right.type, "float") == 0) {
            if (strcmp(node->node_type, "<") == 0)
                result.value.bool_val = left.value.float_val < right.value.float_val;
            else if (strcmp(node->node_type, ">") == 0)
                result.value.bool_val = left.value.float_val > right.value.float_val;
            else if (strcmp(node->node_type, "<=") == 0)
                result.value.bool_val = left.value.float_val <= right.value.float_val;
            else
                result.value.bool_val = left.value.float_val >= right.value.float_val;
        } else {
            fprintf(stderr, "Error: Invalid operands for comparison\n");
            exit(1);
        }
    }
    else if (strcmp(node->node_type, "AND") == 0 || strcmp(node->node_type, "OR") == 0) {
        if (strcmp(left.type, "bool") != 0 || strcmp(right.type, "bool") != 0) {
            fprintf(stderr, "Error: Logical operators require boolean operands\n");
            exit(1);
        }
        result.type = strdup("bool");
        if (strcmp(node->node_type, "AND") == 0)
            result.value.bool_val = left.value.bool_val && right.value.bool_val;
        else
            result.value.bool_val = left.value.bool_val || right.value.bool_val;
    } else {
        fprintf(stderr, "Error: Unknown binary operator %s\n", node->node_type);
        exit(1);
    }

    return result;
}

Variable evaluate_unary_op(ast_node* node) {
    Variable operand = evaluate_expression(node->left);
    Variable result;

    if (strcmp(node->node_type, "NOT") == 0) {
        if (strcmp(operand.type, "bool") != 0) {
            fprintf(stderr, "Error: NOT operator requires boolean\n");
            exit(1);
        }
        result.type = strdup("bool");
        result.value.bool_val = !operand.value.bool_val;
    }
    else if (strcmp(node->node_type, "UMINUS") == 0) {
        if (strcmp(operand.type, "int") == 0) {
            result.type = strdup("int");
            result.value.int_val = -operand.value.int_val;
        }
        else if (strcmp(operand.type, "float") == 0) {
            result.type = strdup("float");
            result.value.float_val = -operand.value.float_val;
        }
        else {
            fprintf(stderr, "Error: UMINUS requires numeric type\n");
            exit(1);
        }
    }
    else {
        fprintf(stderr, "Error: Unknown unary operator %s\n", node->node_type);
        exit(1);
    }

    return result;
}


Variable evaluate_expression(ast_node* node) {
    if (!node) {
        fprintf(stderr, "Error: Null expression\n");
        exit(1);
    }

    if (strcmp(node->node_type, "ID") == 0) {
        return evaluate_identifier(node);
    }
    else if (strcmp(node->node_type, "INT") == 0 || 
             strcmp(node->node_type, "FLOAT") == 0 ||
             strcmp(node->node_type, "STRING") == 0 ||
             strcmp(node->node_type, "BOOL") == 0) {
        return evaluate_literal(node);
    }
    else if (strcmp(node->node_type, "+") == 0 ||
             strcmp(node->node_type, "-") == 0 ||
             strcmp(node->node_type, "*") == 0 ||
             strcmp(node->node_type, "/") == 0 ||
             strcmp(node->node_type, "==") == 0 ||
             strcmp(node->node_type, "!=") == 0 ||
             strcmp(node->node_type, "<") == 0 ||
             strcmp(node->node_type, ">") == 0 ||
             strcmp(node->node_type, "<=") == 0 ||
             strcmp(node->node_type, ">=") == 0 ||
             strcmp(node->node_type, "AND") == 0 ||
             strcmp(node->node_type, "OR") == 0) {
        return evaluate_binary_op(node);
    }
    else if (strcmp(node->node_type, "NOT") == 0 ||
             strcmp(node->node_type, "UMINUS") == 0) {
        return evaluate_unary_op(node);
    }
    else if (strcmp(node->node_type, "call") == 0) {
        interpret_func_call(node);
        Variable result;
        result.type = node->type ? strdup(node->type) : strdup("void");
        return result;
    }
    else {
        fprintf(stderr, "Error: Unknown expression type: %s\n", node->node_type);
        exit(1);
    }
}

Variable evaluate_identifier(ast_node* node) {
    Variable* var = find_variable(node->value);
    if (!var) {
        fprintf(stderr, "Error: Variable '%s' not found\n", node->value);
        exit(1);
    }
    Variable result;
    result.type = strdup(var->type);
    result.value = var->value;
    return result;
}

Variable evaluate_literal(ast_node* node) {
    Variable result;
    result.type = strdup(node->type);

    if (strcmp(node->node_type, "INT") == 0) {
        result.value.int_val = atoi(node->value);
    }
    else if (strcmp(node->node_type, "FLOAT") == 0) {
        result.value.float_val = atof(node->value);
    }
    else if (strcmp(node->node_type, "STRING") == 0) {
        result.value.string_val = node->value ? strdup(node->value) : strdup("");
    }
    else if (strcmp(node->node_type, "BOOL") == 0) {
        result.value.bool_val = (strcmp(node->value, "true") == 0);
    }

    return result;
}

Variable* find_variable(const char* name) {
    for (int i = var_count - 1; i >= 0; i--) {
        if (strcmp(variables[i].name, name) == 0) {
            return &variables[i];
        }
    }
    return NULL;
}

Variable create_variable(const char* type, const char* value_str) {
    Variable var;
    var.type = strdup(type);
    if (value_str) {
        if (strcmp(type, "int") == 0) {
            var.value.int_val = atoi(value_str);
        } else if (strcmp(type, "float") == 0) {
            var.value.float_val = atof(value_str);
        } else if (strcmp(type, "string") == 0) {
            var.value.string_val = strdup(value_str);
        } else if (strcmp(type, "bool") == 0) {
            var.value.bool_val = (strcmp(value_str, "true") == 0);
        }
    } else {
        if (strcmp(type, "int") == 0) var.value.int_val = 0;
        else if (strcmp(type, "float") == 0) var.value.float_val = 0.0;
        else if (strcmp(type, "string") == 0) var.value.string_val = strdup("");
        else if (strcmp(type, "bool") == 0) var.value.bool_val = false;
    }
    return var;
}

void print_variable(Variable var) {
    if (strcmp(var.type, "int") == 0) printf("%d\n", var.value.int_val);
    else if (strcmp(var.type, "float") == 0) printf("%f\n", var.value.float_val);
    else if (strcmp(var.type, "string") == 0) printf("%s\n", var.value.string_val);
    else if (strcmp(var.type, "bool") == 0) printf("%s\n", var.value.bool_val ? "true" : "false");
}
