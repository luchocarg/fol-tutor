#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdlib.h>
#include "transform.h"
#include "core/ast.h"

static int alpha_counter = 0;
void transform_reset_alpha_counter(void) { alpha_counter = 0; }

static char* make_fresh_name(int id) {
    char buf[40];
    int pos = 39;
    buf[pos--] = '\0';
    if (id == 0) { buf[pos--] = '0'; }
    else {
        int tmp = id;
        while (tmp > 0 && pos > 4) {
            buf[pos--] = (char)((tmp % 10) + '0');
            tmp /= 10;
        }
    }
    buf[pos--] = '_';
    buf[pos--] = 'x';
    buf[pos] = '?';
    return strdup(&buf[pos]);
}

static const char* lookup_var(VarMap* map, const char* name) {
    for (; map; map = map->next) {
        if (strcmp(map->original, name) == 0) return map->replacement;
    }
    return name;
}

static void rename_in_term(Term* t, VarMap* map) {
    if (!t) return;

    if (t->type == TERM_VARIABLE) {
        const char* new_name = lookup_var(map, t->name);
        if (new_name != t->name) {
            free(t->name);
            t->name = strdup(new_name);
        }
    }

    for (int i = 0; i < t->arity; i++) {
        rename_in_term(t->args[i], map);
    }
}

void transform_alpha_rename(ASTNode* n, VarMap* map) {
    if (!n) return;

    if (n->type == NODE_QUANTIFIER) {
        char* fresh = make_fresh_name(++alpha_counter);
        char* old_name = n->name;
        
        VarMap new_scope = { old_name, fresh, map };
        
        n->name = fresh;
        transform_alpha_rename(n->left, &new_scope);

        if (old_name) free(old_name);
    } 
    else if (n->type == NODE_ATOM) {
        const char* new_atom_name = lookup_var(map, n->name);
        if (new_atom_name != n->name) {
            free(n->name);
            n->name = strdup(new_atom_name);
        }

        for (int i = 0; i < n->arity; i++) {
            rename_in_term(n->terms[i], map);
        }
    } 
    else {
        transform_alpha_rename(n->left, map);
        transform_alpha_rename(n->right, map);
    }
}