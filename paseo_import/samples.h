#ifndef SAMPLES_H
#define SAMPLES_H
#include <stddef.h>

struct samples_vec {
    size_t sample_count;
    size_t sample_max;
    int *samples;
    long *sample_timestamps;
};

void NewSamples(struct samples_vec *s, const size_t start_size);

void AddSample(struct samples_vec *s, const int sample, const long timestamp);

void FreeSamples(struct samples_vec *s);

#endif //SAMPLES_H
