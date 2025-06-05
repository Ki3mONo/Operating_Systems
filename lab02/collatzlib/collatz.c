#include "collatz.h"
#include <stdlib.h>

int collatz_conjecture(int input) {
    if (input % 2 == 0)
        return input / 2;
    else
        return 3 * input + 1;
}

int test_collatz_convergence(int input, int max_iter, int *steps) {
    int count = 0;
    while (input != 1 && count < max_iter) {
        steps[count] = input;
        input = collatz_conjecture(input);
        count++;
    }
    if (input == 1) {
        steps[count] = 1;
        return count + 1;
    }
    return 0;
}