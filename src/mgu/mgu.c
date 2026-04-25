#define _POSIX_C_SOURCE 200809L
#include "mgu.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "core/parser.h"
#include "core/lexer.h"

Substitution* parse_substitution_string(const char* input) {
    if (!input || strcmp(input, "auto") == 0) return NULL;
    
    Substitution* head = NULL;
    char* str = strdup(input);
    char* token = strtok(str, ";");
    
    while (token) {
        char* eq = strchr(token, '=');
        if (eq) {
            *eq = '\0';
            char* var_name = token;
            char* term_str = eq + 1;
            
            while(*var_name == ' ') ++var_name;
            while(*term_str == ' ') ++term_str;
            
            char* ve = var_name + strlen(var_name) - 1;
            while(ve > var_name && *ve == ' ') { *ve = '\0'; ve--; }
            
            Lexer l = { .source = term_str, .cursor = 0 };
            SymbolTable* st = create_symbol_table();
            Parser p = { .l = &l, .st = st };
            
            Term* replacement = parse_term(&p);
            
            if (replacement) {
                Substitution* sub = malloc(sizeof(Substitution));
                sub->var_name = strdup(var_name);
                sub->term = replacement;
                sub->next = head;
                head = sub;
            }
            free_symbol_table(st);
        }
        token = strtok(NULL, ";");
    }
    free(str);
    return head;
}

Term* get_substitution(Substitution* s, const char* var_name) {
    while (s) {
        if (strcmp(s->var_name, var_name) == 0) return s->term;
        s = s->next;
    }
    return NULL;
}

static bool occurs_check(const char* var_name, Term* t, Substitution* s) {
    if (!t) return false;
    
    Term* sub = get_substitution(s, t->name);
    if (sub && t->type == TERM_VARIABLE) {
        return occurs_check(var_name, sub, s);
    }

    if (t->type == TERM_VARIABLE && strcmp(var_name, t->name) == 0) return true;

    for (int i = 0; i < t->arity; i++) {
        if (occurs_check(var_name, t->args[i], s)) return true;
    }
    return false;
}

static void apply_sub_to_term(Term** t_ptr, const char* var_name, Term* replacement) {
    Term* t = *t_ptr;
    if (!t) return;

    if (t->type == TERM_VARIABLE && strcmp(t->name, var_name) == 0) {
        free_term(t);
        *t_ptr = copy_term(replacement);
        return;
    }

    for (int i = 0; i < t->arity; i++) {
        apply_sub_to_term(&t->args[i], var_name, replacement);
    }
}

static void add_substitution(Substitution** s, const char* var_name, Term* term) {
    Substitution* curr = *s;
    while (curr) {
        apply_sub_to_term(&curr->term, var_name, term);
        curr = curr->next;
    }

    Substitution* new_sub = malloc(sizeof(Substitution));
    new_sub->var_name = strdup(var_name);
    new_sub->term = copy_term(term);
    new_sub->next = *s;
    *s = new_sub;
}

static void trace_step(char* trace_buf, const char* msg, Term* t1, Term* t2) {
    if (!trace_buf) return;
    char* p = trace_buf + strlen(trace_buf);
    p += sprintf(p, "- %s", msg);
    if (t1) {
        p += sprintf(p, ": ");
        term_to_formula(t1, p);
        p += strlen(p);
    }
    if (t2) {
        p += sprintf(p, " vs ");
        term_to_formula(t2, p);
        p += strlen(p);
    }
    sprintf(p, "\n");
}

static bool unify_terms_internal(Term* t1, Term* t2, Substitution** s, char* trace_buf) {
    while (t1->type == TERM_VARIABLE) {
        Term* sub = get_substitution(*s, t1->name);
        if (!sub) break;
        t1 = sub;
    }
    while (t2->type == TERM_VARIABLE) {
        Term* sub = get_substitution(*s, t2->name);
        if (!sub) break;
        t2 = sub;
    }

    if (t1->type == TERM_VARIABLE && t2->type == TERM_VARIABLE && strcmp(t1->name, t2->name) == 0) {
        return true;
    }

    if (t1->type == TERM_VARIABLE) {
        if (occurs_check(t1->name, t2, *s)) {
            trace_step(trace_buf, "Fallo de Occurs-check", t1, t2);
            return false;
        }
        trace_step(trace_buf, "Asignación", t1, t2);
        add_substitution(s, t1->name, t2);
        return true;
    }

    if (t2->type == TERM_VARIABLE) {
        if (occurs_check(t2->name, t1, *s)) {
            trace_step(trace_buf, "Fallo de Occurs-check", t2, t1);
            return false;
        }
        trace_step(trace_buf, "Asignación", t2, t1);
        add_substitution(s, t2->name, t1);
        return true;
    }

    if (strcmp(t1->name, t2->name) != 0) {
        trace_step(trace_buf, "Conflicto de símbolos", t1, t2);
        return false;
    }
    if (t1->arity != t2->arity) {
        trace_step(trace_buf, "Conflicto de aridad", t1, t2);
        return false;
    }

    for (int i = 0; i < t1->arity; i++) {
        if (!unify_terms_internal(t1->args[i], t2->args[i], s, trace_buf)) return false;
    }

    return true;
}

bool unify_terms(Term* t1, Term* t2, Substitution** s) {
    return unify_terms_internal(t1, t2, s, NULL);
}

void free_substitution(Substitution* s) {
    while (s) {
        Substitution* next = s->next;
        free(s->var_name);
        free_term(s->term);
        free(s);
        s = next;
    }
}

Substitution* calculate_mgu(Literal* l1, Literal* l2, bool* success) {
    *success = true;
    if (strcmp(l1->predicate_name, l2->predicate_name) != 0 || l1->arity != l2->arity) {
        *success = false;
        return NULL;
    }

    Substitution* s = NULL;
    for (int i = 0; i < l1->arity; i++) {
        if (!unify_terms_internal(l1->args[i], l2->args[i], &s, NULL)) {
            *success = false;
            free_substitution(s);
            return NULL;
        }
    }
    return s;
}

void calculate_mgu_trace(Literal* l1, Literal* l2, char* trace_buf) {
    trace_buf[0] = '\0';
    char* p = trace_buf;
    if (strcmp(l1->predicate_name, l2->predicate_name) != 0) {
        sprintf(p, "Error: Nombres de predicados distintos.\n");
        return;
    }
    if (l1->arity != l2->arity) {
        sprintf(p, "Error: Aridad distinta.\n");
        return;
    }

    Substitution* s = NULL;
    bool success = true;
    for (int i = 0; i < l1->arity; i++) {
        if (!unify_terms_internal(l1->args[i], l2->args[i], &s, trace_buf)) {
            success = false;
            break;
        }
    }

    p = trace_buf + strlen(trace_buf);
    if (success) {
        sprintf(p, "Éxito: Unificación completada.\n");
    } else {
        sprintf(p, "Fallo: No se pudo unificar.\n");
    }
    free_substitution(s);
}

Substitution* calculate_simultaneous_mgu(Literal** lits1, int count1, Literal** lits2, int count2, bool* success) {
    *success = true;
    if (count1 == 0 || count2 == 0) {
        *success = false;
        return NULL;
    }
    
    Literal* pivot1 = lits1[0];
    Substitution* s = NULL;
    
    for (int i = 1; i < count1; i++) {
        Literal* curr = lits1[i];
        if (curr->is_negative != pivot1->is_negative || strcmp(curr->predicate_name, pivot1->predicate_name) != 0 || curr->arity != pivot1->arity) {
            *success = false; free_substitution(s); return NULL;
        }
        for (int k = 0; k < pivot1->arity; k++) {
            if (!unify_terms_internal(pivot1->args[k], curr->args[k], &s, NULL)) {
                *success = false; free_substitution(s); return NULL;
            }
        }
    }
    
    for (int i = 0; i < count2; i++) {
        Literal* curr = lits2[i];
        if (curr->is_negative == pivot1->is_negative || strcmp(curr->predicate_name, pivot1->predicate_name) != 0 || curr->arity != pivot1->arity) {
            *success = false; free_substitution(s); return NULL;
        }
        for (int k = 0; k < pivot1->arity; k++) {
            if (!unify_terms_internal(pivot1->args[k], curr->args[k], &s, NULL)) {
                *success = false; free_substitution(s); return NULL;
            }
        }
    }
    
    return s;
}

void apply_substitution_to_term(Term** t_ptr, Substitution* s) {
    Term* t = *t_ptr;
    if (!t || !s) return;

    if (t->type == TERM_VARIABLE) {
        Term* replacement = get_substitution(s, t->name);
        if (replacement) {
            free_term(t);
            *t_ptr = copy_term(replacement);
            return; 
        }
    }

    for (int i = 0; i < t->arity; i++) {
        apply_substitution_to_term(&t->args[i], s);
    }
}

void apply_substitution_to_literal(Literal* l, Substitution* s) {
    if (!l || !s) return;
    for (int i = 0; i < l->arity; i++) {
        apply_substitution_to_term(&l->args[i], s);
    }
}

void apply_substitution_to_clause(Clause* c, Substitution* s) {
    if (!c || !s) return;
    for (int i = 0; i < c->count; i++) {
        apply_substitution_to_literal(c->literals[i], s);
    }
}

void calculate_mgu_string(Literal* l1, Literal* l2, char* output_buffer) {
    output_buffer[0] = '\0';
    bool success;
    Substitution* s = calculate_mgu(l1, l2, &success);

    if (!success) {
        sprintf(output_buffer, "Fail");
        return;
    }

    if (!s) {
        sprintf(output_buffer, "{}");
        return;
    }

    char* p = output_buffer;
    p += sprintf(p, "{");
    Substitution* curr = s;
    while (curr) {
        p += sprintf(p, "%s = ", curr->var_name);
        term_to_formula(curr->term, p);
        p += strlen(p);

        if (curr->next) p += sprintf(p, "; ");
        curr = curr->next;
    }
    sprintf(p, "}");

    free_substitution(s);
}

Clause* create_resolvent(Clause* c1, int index1, Clause* c2, int index2, Substitution* sigma) {
    Clause* res = create_empty_clause();
    res->capacity = (c1->count + c2->count);
    free(res->literals);
    res->literals = malloc((size_t)res->capacity * sizeof(Literal*));
    res->parent1 = c1;
    res->parent2 = c2;

    for (int i = 0; i < c1->count; i++) {
        if (i == index1) continue;
        Literal* lit = copy_literal(c1->literals[i]);
        apply_substitution_to_literal(lit, sigma);
        res->literals[res->count++] = lit;
    }

    for (int i = 0; i < c2->count; i++) {
        if (i == index2) continue;
        Literal* lit = copy_literal(c2->literals[i]);
        apply_substitution_to_literal(lit, sigma);
        res->literals[res->count++] = lit;
    }

    return res;
}

Clause* create_general_resolvent(Clause* c1, int* mask1, int len1, Clause* c2, int* mask2, int len2, Substitution* sigma) {
    Clause* res = create_empty_clause();
    res->capacity = c1->count + c2->count;
    free(res->literals);
    res->literals = malloc((size_t)res->capacity * sizeof(Literal*));
    res->parent1 = c1;
    res->parent2 = c2;

    for (int i = 0; i < c1->count; i++) {
        bool skip = false;
        for (int m = 0; m < len1; m++) if (i == mask1[m]) { skip = true; break; }
        if (skip) continue;
        
        Literal* lit = copy_literal(c1->literals[i]);
        apply_substitution_to_literal(lit, sigma);
        res->literals[res->count++] = lit;
    }

    for (int i = 0; i < c2->count; i++) {
        bool skip = false;
        for (int m = 0; m < len2; m++) if (i == mask2[m]) { skip = true; break; }
        if (skip) continue;
        
        Literal* lit = copy_literal(c2->literals[i]);
        apply_substitution_to_literal(lit, sigma);
        res->literals[res->count++] = lit;
    }
    
    return res;
}

Clause* factor_clause(Clause* c, int i1, int i2, Substitution** out_sigma) {
    if (i1 < 0 || i1 >= c->count || i2 < 0 || i2 >= c->count) return NULL;
    Literal* l1 = c->literals[i1];
    Literal* l2 = c->literals[i2];
    if (l1->is_negative != l2->is_negative || strcmp(l1->predicate_name, l2->predicate_name) != 0 || l1->arity != l2->arity) return NULL;
    Substitution* s = NULL;
    for (int i = 0; i < l1->arity; i++) {
        if (!unify_terms_internal(l1->args[i], l2->args[i], &s, NULL)) { free_substitution(s); return NULL; }
    }
    if (out_sigma) *out_sigma = s;
    else free_substitution(s);

    Clause* res = create_empty_clause();
    res->capacity = c->count;
    free(res->literals);
    res->literals = malloc((size_t)res->capacity * sizeof(Literal*));
    res->parent1 = c;
    res->parent2 = NULL;

    for (int i = 0; i < c->count; i++) {
        if (i == i2) continue;
        Literal* lit = copy_literal(c->literals[i]);
        if (s) apply_substitution_to_literal(lit, s);
        res->literals[res->count++] = lit;
    }
    return res;
}

typedef struct {
    char base[256];
    int max_index;
} VarMaxIndex;

static bool is_digit_char(char c) { return c >= '0' && c <= '9'; }

static void update_max_index(VarMaxIndex* arr, int* arr_len, const char* var_name) {
    const char* underscore = strrchr(var_name, '_');
    int index = 0;
    char base[256] = "";
    if (underscore) {
        bool is_num = true;
        const char* p = underscore + 1;
        if (*p == '\0') is_num = false;
        while (*p) { if (!is_digit_char(*p)) { is_num = false; break; } p++; }
        if (is_num) {
            index = atoi(underscore + 1);
            strncpy(base, var_name, (size_t)(underscore - var_name));
            base[underscore - var_name] = '\0';
        } else strcpy(base, var_name);
    } else strcpy(base, var_name);

    for (int i = 0; i < *arr_len; i++) {
        if (strcmp(arr[i].base, base) == 0) {
            if (index > arr[i].max_index) arr[i].max_index = index;
            return;
        }
    }
    if (*arr_len < 128) {
        strcpy(arr[*arr_len].base, base);
        arr[*arr_len].max_index = index;
        (*arr_len)++;
    }
}

static void collect_max_indices_term(Term* t, VarMaxIndex* arr, int* arr_len) {
    if (!t) return;
    if (t->type == TERM_VARIABLE) update_max_index(arr, arr_len, t->name);
    for (int i = 0; i < t->arity; i++) collect_max_indices_term(t->args[i], arr, arr_len);
}

static void collect_max_indices_clause(Clause* c, VarMaxIndex* arr, int* arr_len) {
    for (int i = 0; i < c->count; i++) {
        for (int k = 0; k < c->literals[i]->arity; k++) collect_max_indices_term(c->literals[i]->args[k], arr, arr_len);
    }
}


static void apply_max_index_term(Term* t, VarMaxIndex* arr, int arr_len) {
    if (!t) return;
    if (t->type != TERM_VARIABLE) {
        for (int i = 0; i < t->arity; i++) apply_max_index_term(t->args[i], arr, arr_len);
        return;
    }
    const char* underscore = strrchr(t->name, '_');
    char base[256] = "";
    if (underscore) {
        bool is_num = true;
        const char* p = underscore + 1;
        while (*p) { if (!is_digit_char(*p)) { is_num = false; break; } p++; }
        if (is_num) { strncpy(base, t->name, (size_t)(underscore - t->name)); base[underscore - t->name] = '\0'; }
        else strcpy(base, t->name);
    } else strcpy(base, t->name);
    int target_index = 1;
    for (int i = 0; i < arr_len; i++) { if (strcmp(arr[i].base, base) == 0) { target_index = arr[i].max_index + 1; break; } }
    
    char new_name[512];
    sprintf(new_name, "%s_%d", base, target_index);
    free(t->name); t->name = strdup(new_name);
}

void standardize_apart_clause(Clause* target, Clause* context) {
    VarMaxIndex max_arr[128];
    int arr_len = 0;
    collect_max_indices_clause(context, max_arr, &arr_len);
    collect_max_indices_clause(target, max_arr, &arr_len);
    for (int i = 0; i < target->count; i++) {
        for (int k = 0; k < target->literals[i]->arity; k++) apply_max_index_term(target->literals[i]->args[k], max_arr, arr_len);
    }
}

void calculate_simultaneous_mgu_string(Substitution* s, char* output_buffer) {
    output_buffer[0] = '\0';
    if (!s) { sprintf(output_buffer, "{}"); return; }
    char* p = output_buffer;
    p += sprintf(p, "{");
    Substitution* curr = s;
    while (curr) {
        p += sprintf(p, "%s = ", curr->var_name);
        term_to_formula(curr->term, p);
        p += strlen(p);
        if (curr->next) p += sprintf(p, "; ");
        curr = curr->next;
    }
    sprintf(p, "}");
}