#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "core/ast.h"
#include "core/utils.h"

ASTNode* copy_ast(ASTNode* src) {
    if (!src) return NULL;
    ASTNode* dst = create_node(src->type);
    dst->op = src->op;
    if (src->name) dst->name = strdup(src->name);
    dst->arity = src->arity;
    if (src->terms) {
        dst->terms = malloc(sizeof(Term*) * (size_t)src->arity);
        for (int i = 0; i < src->arity; i++) dst->terms[i] = copy_term(src->terms[i]);
    }
    dst->left = copy_ast(src->left);
    dst->right = copy_ast(src->right);
    return dst;
}

static char* print_term_name_ptr(const char* name, char* p, char* end) {
    if (name && name[0] == '?') SAFE_APPEND(p, end, "%s", name + 1);
    else if (name) SAFE_APPEND(p, end, "%s", name);
    return p;
}

static char* term_to_formula_ptr(Term* t, char* p, char* end) {
    if (!t || !t->name || p >= end) return p;
    if (t->type == TERM_FUNCTION && t->arity > 0) {
        SAFE_APPEND(p, end, "%s(", t->name);
        for (int i = 0; i < t->arity; i++) {
            p = term_to_formula_ptr(t->args[i], p, end);
            if (i < t->arity - 1) SAFE_APPEND(p, end, ", ");
        }
        SAFE_APPEND(p, end, ")");
    } else {
        SAFE_APPEND(p, end, "%s", t->name);
    }
    return p;
}

void term_to_formula(Term* t, char* buf, size_t size) {
    if (size == 0) return;
    buf[0] = '\0';
    term_to_formula_ptr(t, buf, buf + size);
}

static char* term_to_json_ptr(Term* t, char* p, char* end) {
    if (!t || !t->name || p >= end) return p;
    const char* display = (t->name[0] == '?') ? t->name + 1 : t->name;
    const char* type_str = (t->type == TERM_VARIABLE) ? "var" :
                           (t->arity > 0)             ? "func" : "const";

    SAFE_APPEND(p, end, "{\"name\":\"%s\",\"type\":\"%s\"", display, type_str);

    if (t->arity > 0) {
        SAFE_APPEND(p, end, ",\"args\":[");
        for (int i = 0; i < t->arity; i++) {
            p = term_to_json_ptr(t->args[i], p, end);
            if (i < t->arity - 1) SAFE_APPEND(p, end, ",");
        }
        SAFE_APPEND(p, end, "]");
    }
    SAFE_APPEND(p, end, "}");
    return p;
}

void term_to_json(Term* t, char* buf, size_t size) {
    if (size == 0) return;
    buf[0] = '\0';
    term_to_json_ptr(t, buf, buf + size);
}

static char* ast_to_formula_impl(ASTNode* n, char* p, char* end, int is_root) {
    if (!n || p >= end) return p;
    switch (n->type) {
        case NODE_ATOM:
            if (n->name) SAFE_APPEND(p, end, "%s", n->name);
            if (n->arity > 0) {
                SAFE_APPEND(p, end, "(");
                for (int i = 0; i < n->arity; i++) {
                    p = term_to_formula_ptr(n->terms[i], p, end);
                    if (i < n->arity - 1) SAFE_APPEND(p, end, ", ");
                }
                SAFE_APPEND(p, end, ")");
            }
            break;
        case NODE_UNARY:
            SAFE_APPEND(p, end, "¬");
            p = ast_to_formula_impl(n->left, p, end, 0);
            break;
        case NODE_BINARY:
            if (!is_root) SAFE_APPEND(p, end, "(");
            p = ast_to_formula_impl(n->left, p, end, 0);
            {
                const char* op_str = (n->op == TOKEN_AND) ? " ∧ " :
                                     (n->op == TOKEN_OR)  ? " ∨ " : " ⇒ ";
                SAFE_APPEND(p, end, "%s", op_str);
            }
            p = ast_to_formula_impl(n->right, p, end, 0);
            if (!is_root) SAFE_APPEND(p, end, ")");
            break;
        case NODE_QUANTIFIER:
            SAFE_APPEND(p, end, "%s%s.", (n->op == TOKEN_FORALL ? "∀" : "∃"), (n->name ? n->name : ""));
            p = ast_to_formula_impl(n->left, p, end, 0);
            break;
        default: break;
    }
    return p;
}

void ast_to_formula(ASTNode* n, char* buf, size_t size) {
    if (size == 0) return;
    buf[0] = '\0';
    ast_to_formula_impl(n, buf, buf + size, 1);
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

Term* copy_term(Term* src) {
    if (!src) return NULL;
    Term* dst = create_term(src->type);
    if (!dst) return NULL;
    if (src->name) dst->name = strdup(src->name);
    dst->arity = src->arity;
    if (src->arity > 0 && src->args) {
        dst->args = (Term**)malloc(sizeof(Term*) * (size_t)src->arity);
        for (int i = 0; i < src->arity; i++) dst->args[i] = copy_term(src->args[i]);
    } else dst->args = NULL;
    return dst;
}

Term* create_variable_term(const char* name) {
    Term* t = create_term(TERM_VARIABLE);
    if (t && name) t->name = strdup(name);
    return t;
}

void free_term(Term* t) {
    if (!t) return;
    if (t->args) {
        for (int i = 0; i < t->arity; i++) free_term(t->args[i]);
        free(t->args);
    }
    if (t->name) free(t->name);
    free(t);
}

static char* term_to_sexpr_ptr(Term* t, char* p, char* end) {
    if (!t || p >= end) return p;
    if (t->type == TERM_FUNCTION && t->arity > 0) {
        SAFE_APPEND(p, end, "(");
        p = print_term_name_ptr(t->name, p, end);
        for (int i = 0; i < t->arity; i++) {
            SAFE_APPEND(p, end, " ");
            p = term_to_sexpr_ptr(t->args[i], p, end);
        }
        SAFE_APPEND(p, end, ")");
    } else {
        p = print_term_name_ptr(t->name, p, end);
    }
    return p;
}

void term_to_sexpr(Term* t, char* buf, size_t size) {
    if (size == 0) return;
    buf[0] = '\0';
    term_to_sexpr_ptr(t, buf, buf + size);
}

ASTNode* create_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    node->type = type;
    node->op = TOKEN_ERROR;
    node->left = node->right = NULL;
    node->name = NULL;
    node->terms = NULL;
    node->arity = 0;
    return node;
}

void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    if (node->terms) {
        for (int i = 0; i < node->arity; i++) free_term(node->terms[i]);
        free(node->terms);
    }
    if (node->name) free(node->name);
    free(node);
}

static char* ast_to_sexpr_ptr(ASTNode* n, char* p, char* end) {
    if (!n || p >= end) return p;
    switch (n->type) {
        case NODE_ATOM:
            if (n->arity > 0) SAFE_APPEND(p, end, "(");
            if (n->name) SAFE_APPEND(p, end, "%s", n->name);
            for (int i = 0; i < n->arity; i++) {
                SAFE_APPEND(p, end, " ");
                p = term_to_sexpr_ptr(n->terms[i], p, end);
            }
            if (n->arity > 0) SAFE_APPEND(p, end, ")");
            break;
        case NODE_QUANTIFIER:
            SAFE_APPEND(p, end, "(%s ", n->op == TOKEN_FORALL ? "∀" : "∃");
            if (n->name) p = print_term_name_ptr(n->name, p, end);
            SAFE_APPEND(p, end, " ");
            p = ast_to_sexpr_ptr(n->left, p, end);
            SAFE_APPEND(p, end, ")");
            break;
        case NODE_UNARY:
            SAFE_APPEND(p, end, "(¬ ");
            p = ast_to_sexpr_ptr(n->left, p, end);
            SAFE_APPEND(p, end, ")");
            break;
        case NODE_BINARY: {
            const char* op_str = (n->op == TOKEN_AND) ? "∧" :
                                 (n->op == TOKEN_OR)  ? "∨" : "⇒";
            SAFE_APPEND(p, end, "(%s ", op_str);
            p = ast_to_sexpr_ptr(n->left, p, end);
            SAFE_APPEND(p, end, " ");
            p = ast_to_sexpr_ptr(n->right, p, end);
            SAFE_APPEND(p, end, ")");
            break;
        }
        case NODE_FALSUM:
            SAFE_APPEND(p, end, "⊥");
            break;
        default: break;
    }
    return p;
}

void ast_to_sexpr(ASTNode* n, char* buf, size_t size) {
    if (size == 0) return;
    buf[0] = '\0';
    ast_to_sexpr_ptr(n, buf, buf + size);
}

static char* ast_to_json_ptr(ASTNode* n, char* p, char* end) {
    if (!n || p >= end) return p;
    SAFE_APPEND(p, end, "{\"type\":\"");
    switch (n->type) {
        case NODE_ATOM:
            SAFE_APPEND(p, end, "atom\",\"name\":\"%s\"", n->name ? n->name : "");
            if (n->arity > 0) {
                SAFE_APPEND(p, end, ",\"terms\":[");
                for (int i = 0; i < n->arity; i++) {
                    p = term_to_json_ptr(n->terms[i], p, end);
                    if (i < n->arity - 1) SAFE_APPEND(p, end, ",");
                }
                SAFE_APPEND(p, end, "]");
            }
            break;
        case NODE_QUANTIFIER:
            SAFE_APPEND(p, end, "quantifier\",\"op\":\"%s\",\"var\":\"%s\",\"body\":", 
                n->op == TOKEN_FORALL ? "forall" : "exists", n->name ? n->name : "");
            p = ast_to_json_ptr(n->left, p, end);
            break;
        case NODE_UNARY:
            SAFE_APPEND(p, end, "unary\",\"op\":\"not\",\"body\":");
            p = ast_to_json_ptr(n->left, p, end);
            break;
        case NODE_BINARY:
            SAFE_APPEND(p, end, "binary\",\"op\":\"%s\",\"left\":", 
                n->op == TOKEN_AND ? "and" : n->op == TOKEN_OR ? "or" : "implies");
            p = ast_to_json_ptr(n->left, p, end);
            SAFE_APPEND(p, end, ",\"right\":");
            p = ast_to_json_ptr(n->right, p, end);
            break;
        case NODE_FALSUM:
            SAFE_APPEND(p, end, "falsum\"");
            break;
    }
    SAFE_APPEND(p, end, "}");
    return p;
}

void ast_to_json(ASTNode* n, char* buf, size_t size) {
    if (size == 0) return;
    buf[0] = '\0';
    ast_to_json_ptr(n, buf, buf + size);
}

ASTNode* create_unary_node(TokenType op, ASTNode* child) {
    ASTNode* node = create_node(NODE_UNARY);
    if (!node) return NULL;
    node->op = op;
    node->left = child;
    node->right = NULL;
    return node;
}
