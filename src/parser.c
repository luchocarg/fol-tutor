#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdlib.h>
#include "parser.h"

ASTNode* parse_formula(Lexer* l, SymbolTable* st) {
    ASTNode* node = parse_implication(l, st);
    if (!node) return NULL;
    
    Token next = peek_token(l);
    if (next.type != TOKEN_EOF) {
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
        if (var.type != TOKEN_ID_UPPER) {
            return NULL;
        }

        Token dot = get_next_token(l);
        if (dot.type != TOKEN_DOT) {
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
            free_ast(node);
            return NULL;
        }
        return node;
    }

    if (t.type == TOKEN_FALSUM) {
        get_next_token(l);
        return create_node(NODE_FALSUM);
    }

    if (t.type == TOKEN_ID_UPPER) {
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

    if (t.type == TOKEN_ID_LOWER) {
        
        return NULL;
    }

    return NULL; 
}

Term* parse_term(Lexer* l, SymbolTable* st) {
    Token t = get_next_token(l);
    
    if (t.type != TOKEN_ID_LOWER && t.type != TOKEN_ID_UPPER) return NULL;

    Term* term = create_term(t.type == TOKEN_ID_UPPER ? TERM_VARIABLE : TERM_FUNCTION);
    term->name = strndup(t.start, t.length);

    if (t.type == TOKEN_ID_LOWER && peek_token(l).type == TOKEN_LEFT_PARENT) {
        get_next_token(l);
        term->args = parse_term_list(l, &term->arity, st);
        if (get_next_token(l).type != TOKEN_RIGHT_PARENT) {
            free_term(term);
            return NULL; 
        }
    } else {
        term->arity = 0;
        term->args = NULL;
    }

    if (t.type == TOKEN_ID_LOWER) {
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

void term_to_sexpr(Term* t, char* buf) {
    if (!t) return;

    if (t->type == TERM_FUNCTION && t->arity > 0) {
        strcat(buf, "(");
        strcat(buf, t->name);
        for (int i = 0; i < t->arity; i++) {
            strcat(buf, " ");
            term_to_sexpr(t->args[i], buf);
        }
        strcat(buf, ")");
    } else {
        strcat(buf, t->name);
    }
}