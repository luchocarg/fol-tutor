#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "core/parser.h"
#include <stdio.h>

static bool check_depth(Parser* p) {
    if (p->depth > MAX_PARSE_DEPTH) {
        snprintf(p->error_msg, sizeof(p->error_msg), "Maximum recursion depth exceeded");
        return false;
    }
    return true;
}

ASTNode* parse_formula(Parser* p) {
    p->depth++;
    if (!check_depth(p)) return NULL;
    ASTNode* node = parse_implication(p);
    p->depth--;
    return node;
}

ASTNode* parse_implication(Parser* p) {
    p->depth++;
    if (!check_depth(p)) return NULL;
    ASTNode* node = parse_disjunction(p);
    if (!node) { p->depth--; return NULL; }

    if (peek_token(p->l).type == TOKEN_IMPLIES) {
        get_next_token(p->l);
        ASTNode* right = parse_implication(p);
        if (!right) { free_ast(node); p->depth--; return NULL; }
        ASTNode* parent = create_node(NODE_BINARY);
        parent->op = TOKEN_IMPLIES;
        parent->left = node;
        parent->right = right;
        p->depth--;
        return parent;
    }
    p->depth--;
    return node;
}

ASTNode* parse_disjunction(Parser* p) {
    p->depth++;
    if (!check_depth(p)) return NULL;
    ASTNode* node = parse_conjunction(p);
    if (!node) { p->depth--; return NULL; }

    while (peek_token(p->l).type == TOKEN_OR) {
        get_next_token(p->l);
        ASTNode* right = parse_conjunction(p);
        if (!right) { free_ast(node); p->depth--; return NULL; }
        ASTNode* parent = create_node(NODE_BINARY);
        parent->op = TOKEN_OR;
        parent->left = node;
        parent->right = right;
        node = parent;
    }
    p->depth--;
    return node;
}

ASTNode* parse_conjunction(Parser* p) {
    p->depth++;
    if (!check_depth(p)) return NULL;
    ASTNode* node = parse_unary(p);
    if (!node) { p->depth--; return NULL; }

    while (peek_token(p->l).type == TOKEN_AND) {
        get_next_token(p->l);
        ASTNode* right = parse_unary(p);
        if (!right) { free_ast(node); p->depth--; return NULL; }
        ASTNode* parent = create_node(NODE_BINARY);
        parent->op = TOKEN_AND;
        parent->left = node;
        parent->right = right;
        node = parent;
    }
    p->depth--;
    return node;
}

ASTNode* parse_unary(Parser* p) {
    p->depth++;
    if (!check_depth(p)) return NULL;
    Token t = peek_token(p->l);

    if (t.type == TOKEN_FORALL || t.type == TOKEN_EXISTS) {
        get_next_token(p->l);
        
        Token var = get_next_token(p->l);
        if (var.type != TOKEN_IDENT || !var.is_variable) { 
            snprintf(p->error_msg, sizeof(p->error_msg), "Expected variable after quantifier");
            p->depth--; 
            return NULL; 
        }

        if (get_next_token(p->l).type != TOKEN_DOT) { 
            snprintf(p->error_msg, sizeof(p->error_msg), "Expected '.' after quantifier variable");
            p->depth--; 
            return NULL; 
        }

        ASTNode* node = create_node(NODE_QUANTIFIER);
        if (!node) { p->depth--; return NULL; }
        node->op = t.type;
        char* qname = malloc(var.length + 2);
        qname[0] = '?';
        strncpy(qname + 1, var.start, var.length);
        qname[var.length + 1] = '\0';
        node->name = qname;
        node->left = parse_unary(p);
        if (!node->left) { free_ast(node); p->depth--; return NULL; }
        p->depth--;
        return node;
    }

    if (t.type == TOKEN_NOT) {
        get_next_token(p->l);
        ASTNode* node = create_unary_node(TOKEN_NOT, parse_unary(p));
        if (!node || !node->left) { free_ast(node); p->depth--; return NULL; }
        p->depth--;
        return node;
    }

    ASTNode* res = parse_primary(p);
    p->depth--;
    return res;
}

ASTNode* parse_primary(Parser* p) {
    p->depth++;
    if (!check_depth(p)) return NULL;
    Token t = peek_token(p->l);
    if (t.type == TOKEN_ERROR) {
        snprintf(p->error_msg, sizeof(p->error_msg), "Illegal character '%c'", p->l->error_char);
        p->depth--;
        return NULL;
    }

    if (t.type == TOKEN_LEFT_PARENT) {
        get_next_token(p->l);
        ASTNode* node = parse_implication(p);
        if (!node) { p->depth--; return NULL; }
        if (get_next_token(p->l).type != TOKEN_RIGHT_PARENT) { 
            snprintf(p->error_msg, sizeof(p->error_msg), "Missing closing parenthesis ')'");
            free_ast(node); 
            p->depth--; 
            return NULL; 
        }
        p->depth--;
        return node;
    }

    if (t.type == TOKEN_FALSUM) {
        get_next_token(p->l);
        p->depth--;
        return create_node(NODE_FALSUM);
    }

    if (t.type == TOKEN_IDENT && !t.is_variable) {
        Token name_tok = get_next_token(p->l);
        ASTNode* node = create_node(NODE_ATOM);
        if (!node) { p->depth--; return NULL; }
        node->name = strndup(name_tok.start, name_tok.length);

        if (peek_token(p->l).type == TOKEN_LEFT_PARENT) {
            get_next_token(p->l);
            node->terms = parse_term_list(p, &node->arity);
            if (!node->terms) {
                free_ast(node);
                p->depth--;
                return NULL;
            }
            if (get_next_token(p->l).type != TOKEN_RIGHT_PARENT) {
                snprintf(p->error_msg, sizeof(p->error_msg), "Missing ')' after predicate terms");
                free_ast(node);
                p->depth--;
                return NULL;
            }
        } else {
            node->arity = 0;
            node->terms = NULL;
        }

        if (!validate_or_register(p->st, node->name, node->arity, SYM_PREDICATE)) {
            snprintf(p->error_msg, sizeof(p->error_msg), "Arity mismatch for predicate '%s'", node->name);
            free_ast(node);
            p->depth--;
            return NULL;
        }
        p->depth--;
        return node;
    }

    snprintf(p->error_msg, sizeof(p->error_msg), "Unexpected token at primary expression");
    p->depth--;
    return NULL;
}

Term* parse_term(Parser* p) {
    p->depth++;
    if (!check_depth(p)) return NULL;
    Token t = get_next_token(p->l);
    if (t.type != TOKEN_IDENT) { 
        snprintf(p->error_msg, sizeof(p->error_msg), "Expected identifier in term");
        p->depth--; 
        return NULL; 
    }

    TermType type = t.is_variable ? TERM_VARIABLE : TERM_FUNCTION;
    Term* term = create_term(type);
    if (!term) { p->depth--; return NULL; }
    if (t.is_variable) {
        char* vname = malloc(t.length + 2);
        vname[0] = '?';
        strncpy(vname + 1, t.start, t.length);
        vname[t.length + 1] = '\0';
        term->name = vname;
    } else {
        term->name = strndup(t.start, t.length);
    }

    if (type == TERM_FUNCTION && peek_token(p->l).type == TOKEN_LEFT_PARENT) {
        get_next_token(p->l);
        term->args = parse_term_list(p, &term->arity);
        if (!term->args) {
            free_term(term);
            p->depth--;
            return NULL;
        }
        if (get_next_token(p->l).type != TOKEN_RIGHT_PARENT) {
            snprintf(p->error_msg, sizeof(p->error_msg), "Missing ')' after function arguments");
            free_term(term);
            p->depth--;
            return NULL;
        }
    } else {
        term->arity = 0;
        term->args = NULL;
    }

    if (type == TERM_FUNCTION) {
        if (!validate_or_register(p->st, term->name, term->arity, SYM_FUNCTION)) {
            snprintf(p->error_msg, sizeof(p->error_msg), "Arity mismatch for function '%s'", term->name);
            free_term(term);
            p->depth--;
            return NULL;
        }
    }

    p->depth--;
    return term;
}

Term** parse_term_list(Parser* p, int* count) {
    int capacity = 4;
    int n = 0;
    Term** list = malloc(sizeof(Term*) * (size_t)capacity);
    if (!list) return NULL;

    while (1) {
        if (n >= MAX_ARITY) {
            for (int i = 0; i < n; i++) free_term(list[i]);
            free(list);
            return NULL;
        }

        Term* t = parse_term(p);
        if (!t) {
            for (int i = 0; i < n; i++) free_term(list[i]);
            free(list);
            return NULL;
        }

        if (n >= capacity) {
            capacity *= 2;
            Term** tmp = realloc(list, sizeof(Term*) * (size_t)capacity);
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