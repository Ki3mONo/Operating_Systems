#include <stdio.h>
#include <stdlib.h>

#ifndef DYNAMIC_LOAD
    #include "collatzlib/collatz.h"
#endif // DYNAMIC_LOAD

#ifdef DYNAMIC_LOAD
    #include "dlfcn.h"

    int (*collatz_conjecture)(int input);
    int (*test_collatz_convergence)(int input, int max_iter, int *steps);

    int test_dynamic(int number, int max_iter) {
        void *handle = dlopen("./collatzlib/libcollatz.so", RTLD_LAZY);
        if (!handle) {
            fprintf(stderr, "%s\n", dlerror());
            return 1;
        }


        collatz_conjecture = (int (*)(int))dlsym(handle, "collatz_conjecture");
        test_collatz_convergence = (int (*)(int, int, int *))dlsym(handle, "test_collatz_convergence");

        if (dlerror() != NULL) {
            fprintf(stderr, "Error loading functions\n");
            dlclose(handle);
            return 1; 
        }


        int *steps = malloc(max_iter * sizeof(int));
        if (steps == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            dlclose(handle);
            return 1; 
        }

        int count = test_collatz_convergence(number, max_iter, steps);
        if (count > 0) {
            printf("Collatz sequence for %d (steps: %d): ", number, count);
            for (int i = 0; i < count; i++) {
                printf("%d ", steps[i]);
            }
            printf("\n");
        } else {
            printf("Collatz sequence for %d did not converge in %d steps.\n", number, max_iter);
        }

        free(steps);
        dlclose(handle);

        return 0;
    }
#endif // DYNAMIC_LOAD

#ifndef DYNAMIC_LOAD
void test_static_and_shared(int number, int max_iter);

void test_static_and_shared(int number, int max_iter) {
    int steps[100];
    int count = test_collatz_convergence(number, max_iter, steps);
    if (count > 0) {
        printf("Collatz sequence for %d (steps: %d): ", number, count);
        for (int i = 0; i < count; i++) {
            printf("%d ", steps[i]);
        }
        printf("\n");
    } else {
        printf("Collatz sequence for %d did not converge in %d steps.\n", number, max_iter);
    }
}
#endif // DYNAMIC_LOAD

int main() {
    int numbers[] = {6, 11, 19, 27};
    int max_iter = 100;

    #ifdef DYNAMIC_LOAD
    for (int i = 0; i < 4; i++) {
        test_dynamic(numbers[i], max_iter);
    }
    #else
    for (int i = 0; i < 4; i++) {
        test_static_and_shared(numbers[i], max_iter);
    }
    #endif

    return 0;  // Sukces
}
