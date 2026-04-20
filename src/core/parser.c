#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdlib.h>
#include "core/parser.h"

ASTNode* parse_formula(Parser* p) {
    ASTNode* node = parse_implication(p);
    if (!node) return NULL;
    
    if (peek_token(p->l).type != TOKEN_EOF) {}

    return node;
}

ASTNode* parse_implication(Parser* p) {
    ASTNode* node = parse_disjunction(p);
    if (!node) return NULL;

    if (peek_token(p->l).type == TOKEN_IMPLIES) {
        get_next_token(p->l);

        ASTNode* right = parse_implication(p);
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

ASTNode* parse_disjunction(Parser* p) {
    ASTNode* node = parse_conjunction(p);
    if (!node) return NULL;

    while (peek_token(p->l).type == TOKEN_OR) {
        get_next_token(p->l);
        
        ASTNode* right = parse_conjunction(p);
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

ASTNode* parse_conjunction(Parser* p) {
    ASTNode* node = parse_unary(p);
    if (!node) return NULL;

    while (peek_token(p->l).type == TOKEN_AND) {
        get_next_token(p->l);
        
        ASTNode* right = parse_unary(p);
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

ASTNode* parse_unary(Parser* p) {
    Token t = peek_token(p->l);

    if (t.type == TOKEN_FORALL || t.type == TOKEN_EXISTS) {
        get_next_token(p->l);
        
        Token var = get_next_token(p->l);
        if (var.type != TOKEN_ID_UPPER) return NULL;

        if (get_next_token(p->l).type != TOKEN_DOT) return NULL;

        ASTNode* node = create_node(NODE_QUANTIFIER);
        if (!node) return NULL;

        node->op = t.type;
        node->name = strndup(var.start, var.length);
        
        node->left = parse_unary(p); 
        if (!node->left) {
            free_ast(node);
            return NULL;
        }
        return node;
    }

    if (t.type == TOKEN_NOT) {
        get_next_token(p->l);
        ASTNode* node = create_unary_node(TOKEN_NOT, parse_unary(p));
        if (!node || !node->left) {
            free_ast(node);
            return NULL;
        }
        return node;
    }

    return parse_primary(p);
}

ASTNode* parse_primary(Parser* p) {
    Token t = peek_token(p->l);

    if (t.type == TOKEN_LEFT_PARENT) {
        get_next_token(p->l);
        ASTNode* node = parse_implication(p);
        if (!node) return NULL;
        if (get_next_token(p->l).type != TOKEN_RIGHT_PARENT) {
            free_ast(node);
            return NULL;
        }
        return node;
    }

    if (t.type == TOKEN_FALSUM) {
        get_next_token(p->l);
        return create_node(NODE_FALSUM);
    }

    if (t.type == TOKEN_ID_UPPER) {
        Token name_tok = get_next_token(p->l);
        ASTNode* node = create_node(NODE_ATOM);
        if (!node) return NULL;

        node->name = strndup(name_tok.start, name_tok.length);
        
        if (peek_token(p->l).type == TOKEN_LEFT_PARENT) {
            get_next_token(p->l);
            node->terms = parse_term_list(p, &node->arity);
            
            if (!node->terms || get_next_token(p->l).type != TOKEN_RIGHT_PARENT) {
                free_ast(node);
                return NULL;
            }
        } else {
            node->arity = 0;
            node->terms = NULL;
        }
        
        if (!validate_or_register(p->st, node->name, node->arity, SYM_PREDICATE)) {
            free_ast(node);
            return NULL;
        }
        return node;
    }

    return NULL; 
}

Term* parse_term(Parser* p) {
    Token t = get_next_token(p->l);
    if (t.type != TOKEN_ID_LOWER && t.type != TOKEN_ID_UPPER) return NULL;

    TermType type = (t.type == TOKEN_ID_UPPER) ? TERM_VARIABLE : TERM_FUNCTION;
    Term* term = create_term(type);
    if (!term) return NULL;
    term->name = strndup(t.start, t.length);

    if (type == TERM_FUNCTION && peek_token(p->l).type == TOKEN_LEFT_PARENT) {
        get_next_token(p->l);
        term->args = parse_term_list(p, &term->arity);
        if (!term->args || get_next_token(p->l).type != TOKEN_RIGHT_PARENT) {
            free_term(term);
            return NULL; 
        }
    } else {
        term->arity = 0;
        term->args = NULL;
    }

    if (type == TERM_FUNCTION) {
        if (!validate_or_register(p->st, term->name, term->arity, SYM_FUNCTION)) {
            free_term(term);
            return NULL;
        }
    }

    return term;
}

Term** parse_term_list(Parser* p, int* count) {
    int capacity = 4;
    int n = 0;
    Term** list = malloc(sizeof(Term*) * (size_t)capacity);
    if (!list) return NULL;

    while (1) {
        Term* t = parse_term(p);
        if (!t) {
            for (int i = 0; i < n; i++) free_term(list[i]);
            free(list);
            return NULL;
        }

        if (n >= capacity) {
            capacity *= 2;
            Term** tmp = realloc(list, sizeof(Term*) * (size_t)capacity);
            if (!tmp) {}
            list = tmp;
        }
        list[n++] = t;

        if (peek_token(p->l).type == TOKEN_COMMA) {
            get_next_token(p->l);
        } else {
            break;
        }
    }

    *count = n;
    return list;
}