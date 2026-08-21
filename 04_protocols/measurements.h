#ifndef MVERSE_DEMO_MEASUREMENTS_H
#define MVERSE_DEMO_MEASUREMENTS_H

typedef struct Temperature {
    int degrees;
} Temperature;

typedef struct Distance {
    int meters;
} Distance;

static int measurement_from_temperature(Temperature value) {
    return value.degrees;
}

static int measurement_from_distance(Distance value) {
    return value.meters;
}

@impl(measurement, Temperature, measurement_from_temperature)
@impl(measurement, Distance, measurement_from_distance)
@emit_protocol(int, measurement)

#endif
