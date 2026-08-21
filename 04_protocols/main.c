#include <stdio.h>
@import("measurements.h")

int main(void) {
    Temperature temperature = { .degrees = 21 };
    Distance distance = { .meters = 300 };

    printf("temperature: %d\n", @measurement(temperature));
    printf("distance: %d\n", @measurement(distance));
    return 0;
}
