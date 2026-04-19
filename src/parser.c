#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"

int parser_silent_mode = 0;

ASTNode* parse_formula(Lexer* l, SymbolTable* st) {
    ASTNode* node = parse_implication(l, st);
    if (!node) return NULL;
    
    Token next = peek_token(l);
    if (next.type != TOKEN_EOF) {
        fprintf(stderr, "Syntax error: An unexpected token was found at the end.\n");
    }

    return node;


}

ASTNode* parse_implication(Lexer* l, SymbolTable* st) {
    ASTNode* node = parse_disjunction(l, st);
    if (!node) return NULL;

    Token next = peek_token(l);

    if (next.type == TOKEN_IMPLIES) {
        get_next_token(l);

        ASTNode* right = parse_implication(l, st);
        if (!right) { 
            free_ast(node); 
            return NULL; 
        }
        
        ASTNode* parent = create_node(NODE_BINARY);
        parent->op = TOKEN_IMPLIES;
        parent->left = node;
        parent->right = right;
        return parent;
    }

    return node;
}

ASTNode* parse_disjunction(Lexer* l, SymbolTable* st) {
    ASTNode* node = parse_conjunction(l, st);
    if (!node) return NULL;

    while (peek_token(l).type == TOKEN_OR) {
        get_next_token(l);
        
        ASTNode* right = parse_conjunction(l, st);
        if (!right) { 
            free_ast(node); 
            return NULL; 
        }
        
        ASTNode* parent = create_node(NODE_BINARY);
        parent->op = TOKEN_OR;
        parent->left = node;
        parent->right = right;
        node = parent;
    }

    return node;
}

ASTNode* parse_conjunction(Lexer* l, SymbolTable* st) {
    ASTNode* node = parse_unary(l, st);
    if (!node) return NULL;

    while (peek_token(l).type == TOKEN_AND) {
        get_next_token(l);
        
        ASTNode* right = parse_unary(l, st);
        if (!right) { 
            free_ast(node); 
            return NULL; 
        }
        
        ASTNode* parent = create_node(NODE_BINARY);
        parent->op = TOKEN_AND;
        parent->left = node;
        parent->right = right;
        node = parent;
    }

    return node;
}

ASTNode* parse_unary(Lexer* l, SymbolTable* st) {
    Token t = peek_token(l);

    if (t.type == TOKEN_FORALL || t.type == TOKEN_EXISTS) {
        get_next_token(l);
        
        Token var = get_next_token(l);
        if (var.type != TOKEN_UPPER_INDENT) {
            fprintf(stderr, "Syntax error: Expected variable (UPPERCASE) after quantifier.\n");
            return NULL;
        }

        Token dot = get_next_token(l);
        if (dot.type != TOKEN_DOT) {
            fprintf(stderr, "Syntax error: Expected '.' after quantifier variable.\n");
            return NULL;
        }

        ASTNode* node = create_node(NODE_QUANTIFIER);
        if (!node) return NULL;

        node->op = t.type;
        node->name = strndup(var.start, var.length);
        
        node->left = parse_unary(l, st); 
        if (!node->left) {
            free_ast(node);
            return NULL;
        }
        return node;
    }

    if (t.type == TOKEN_NOT) {
        get_next_token(l);
        
        ASTNode* node = create_node(NODE_UNARY);
        if (!node) return NULL;

        node->op = TOKEN_NOT;
        
        node->left = parse_unary(l, st);
        if (!node->left) {
            free_ast(node);
            return NULL;
        }
        return node;
    }

    return parse_primary(l, st);
}

ASTNode* parse_primary(Lexer* l, SymbolTable* st) {
    Token t = peek_token(l);

    if (t.type == TOKEN_LEFT_PARENT) {
        get_next_token(l);
        ASTNode* node = parse_implication(l, st);
        if (!node) return NULL;
        if (get_next_token(l).type != TOKEN_RIGHT_PARENT) {
            fprintf(stderr, "Error: Expected ')'\n");
            free_ast(node);
            return NULL;
        }
        return node;
    }

    if (t.type == TOKEN_FALSUM) {
        get_next_token(l);
        return create_node(NODE_FALSUM);
    }

    if (t.type == TOKEN_UPPER_INDENT) {
        Token name_tok = get_next_token(l);
        ASTNode* node = create_node(NODE_PREDICATE);
        if (!node) return NULL;

        node->name = strndup(name_tok.start, (size_t)name_tok.length);
        
        if (peek_token(l).type == TOKEN_LEFT_PARENT) {
            get_next_token(l);
            node->terms = parse_term_list(l, &node->term_count, st);
            
            if (node->terms == NULL) {
                free_ast(node);
                return NULL;
            }

            if (get_next_token(l).type != TOKEN_RIGHT_PARENT) {
                if (!parser_silent_mode) {
                    fprintf(stderr, "Error: Expected ')' after terms\n");
                }
                free_ast(node);
                return NULL;
            }
        } else {
            node->term_count = 0;
            node->terms = NULL;
        }
        
        if (!validate_or_register(st, node->name, node->term_count, SYM_PREDICATE)) {
            free_ast(node);
            return NULL;
        }
        return node;
    }

    if (t.type == TOKEN_LOWER_INDENT) {
        fprintf(stderr, "Semantic Error: Function or Constant '%.*s' cannot be used as a formula.\n", 
                (int)t.length, t.start);
        return NULL;
    }

    return NULL; 
}

Term* parse_term(Lexer* l, SymbolTable* st) {
    Token t = get_next_token(l);
    
    if (t.type != TOKEN_LOWER_INDENT && t.type != TOKEN_UPPER_INDENT) return NULL;

    Term* term = create_term(t.type == TOKEN_UPPER_INDENT ? TERM_VARIABLE : TERM_FUNCTION);
    term->name = strndup(t.start, t.length);

    if (t.type == TOKEN_LOWER_INDENT && peek_token(l).type == TOKEN_LEFT_PARENT) {
        get_next_token(l);
        term->args = parse_term_list(l, &term->arity, st);
        if (get_next_token(l).type != TOKEN_RIGHT_PARENT) {
            fprintf(stderr, "Syntax error: Expected ')' after function arguments.\n");
            free_term(term);
            return NULL; 
        }
    } else {
        term->arity = 0;
        term->args = NULL;
    }

    if (t.type == TOKEN_LOWER_INDENT) {
        if (!validate_or_register(st, term->name, term->arity, SYM_FUNCTION)) {
            free_term(term);
            return NULL;
        }
    }

    return term;
}

Term** parse_term_list(Lexer* l, int* count, SymbolTable* st) {
    int capacity = 4;
    int n = 0;
    Term** list = malloc(sizeof(Term*) * (size_t)capacity);

    while (1) {
        Term* t = parse_term(l, st);
        if (!t) {
            for (int i = 0; i < n; i++) free_term(list[i]);
            free(list);
            return NULL;
        }

        list[n++] = t;

        if (peek_token(l).type == TOKEN_COMMA) {
            get_next_token(l);
            if (n >= capacity) {
                capacity *= 2;
                list = realloc(list, sizeof(Term*) * (size_t)capacity);
            }
        } else {
            break;
        }
    }

    *count = n;
    return list;
}

ASTNode* create_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type =  type;
    node->op =    TOKEN_ERROR;
    node->left =  NULL;
    node->right = NULL;
    node->name =  NULL;
    node->terms = NULL;
    node->term_count = 0;
    
    return node;
}

Term* create_term(TermType type) {
    Term* t = (Term*)malloc(sizeof(Term));
    if (!t) return NULL;
    
    t->type = type;
    t->name = NULL;
    t->args = NULL;
    t->arity = 0;
    
    return t;
}

void free_term(Term* t) {
    if (!t) return;
    
    if (t->args) {
        for (int i = 0; i < t->arity; i++) {
            free_term(t->args[i]);
        }
        free(t->args);
    }
    
    if (t->name) free(t->name);
    free(t);
}

void free_ast(ASTNode* node) {
    if (!node) return;

    free_ast(node->left);
    free_ast(node->right);

    if (node->terms) {
        for (int i = 0; i < node->term_count; i++) {
            free_term(node->terms[i]);
        }
        free(node->terms);
    }

    if (node->name) free(node->name);
    free(node);
}