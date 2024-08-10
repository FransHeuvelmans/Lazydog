#ifndef VISUAL_H
#define VISUAL_H
#include "samples.h"

// Write a tsv file out which can be used by many tools
// (like both vega-lite and gnuplot)
int write_tsv_file(const struct samples_vec *samples);

#endif //VISUAL_H
