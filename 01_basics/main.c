#include <stdio.h>
#include <time.h>

@def(measure_time(label: char*)) {
    clock_t _start = clock();
    $body
    clock_t _end = clock();
    printf("%s took %f seconds\n", $label, (double)(_end - _start) / CLOCKS_PER_SEC);
}

void do_heavy_math() {
    volatile int sum = 0;
    for (int i = 0; i < 10000000; i++) {
        sum += i;
    }
}

int main() {
    printf("Starting Basics Demo...\n");

    @measure_time("Heavy Math Loop") {
        printf("  Inside the block!\n");
        do_heavy_math();
    }

    printf("Done.\n");
    return 0;
}
