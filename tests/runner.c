#include <stdio.h>
#include "tests.h"

int main(void) {
    printf("=== RUNNING ALL TESTS ===\n\n");

    run_lexer_tests();

    printf("\n=== ALL TESTS PASSED ===\n");
    return 0;
}