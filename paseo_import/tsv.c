#include "tsv.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int write_tsv_file(const struct samples_vec *samples) {
    const time_t current_time = time(NULL);
    char out_path[256];
    const int errc = snprintf(out_path, 256, "paseo_export_%ld.tsv", current_time);
    if (errc < 0) {
        fprintf(stderr, "Failed to create filepath for time %ld\n", current_time);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(out_path, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open the file %s\n", out_path);
        return EXIT_FAILURE;
    }
    // Write the header to the TSV file
    fprintf(file, "time\tsteps\n");

    for (size_t i = 0; i < samples->sample_count; i++) {
        const int s_v = samples->samples[i];
        const long s_i = samples->sample_timestamps[i];
        fprintf(file, "%ld\t%d\n", s_i, s_v);
    }

    fclose(file);
    printf("Data exported to TSV file %s successfully.\n", out_path);

    return EXIT_SUCCESS;
}
