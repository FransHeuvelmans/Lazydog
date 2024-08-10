#include "samples.h"

#include <stdio.h>
#include <stdlib.h>

void NewSamples(struct samples_vec *s, const size_t start_size) {
    s->samples = malloc(start_size * sizeof(int));
    if (s->samples == NULL) {
        fprintf(stderr, "Malloc samples failed for size %lu\n", start_size);
        exit(EXIT_FAILURE);
    }
    s->sample_timestamps = malloc(start_size * sizeof(long));
    if (s->sample_timestamps == NULL) {
        fprintf(stderr, "Malloc timestamps failed for size %lu\n", start_size);
        exit(EXIT_FAILURE);
    }
    s->sample_count = 0;
    s->sample_max = start_size;
}

void AddSample(struct samples_vec *s, const int sample, const long timestamp) {
    if (s->sample_count == s->sample_max) {
        s->sample_max *= 2;
        s->samples = realloc(s->samples, s->sample_max * sizeof(int));
        if (s->samples == NULL) {
            fprintf(stderr, "Remalloc samples failed for size %lu\n", s->sample_max);
            exit(EXIT_FAILURE);
        }
        s->sample_timestamps = realloc(s->sample_timestamps, s->sample_max * sizeof(long));
        if (s->sample_timestamps == NULL) {
            fprintf(stderr, "Remalloc timestamps failed for size %lu\n", s->sample_max);
            exit(EXIT_FAILURE);
        }
    }
    s->samples[s->sample_count] = sample;
    s->sample_timestamps[s->sample_count] = timestamp;
    s->sample_count++;
}

void FreeSamples(struct samples_vec *s) {
    free(s->samples);
    s->samples = NULL;
    free(s->sample_timestamps);
    s->sample_timestamps = NULL;
    s->sample_max = 0;
    s->sample_count = 0;
}
