#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#include "db_ops.h"
#include "tsv.h"

void printHelp() {
    printf("Usage: paseo_import [options] <target_db>\n");
    printf("Converts a paseo exported sqlite db into a Lazydog compatible db and merges paseo-tables in two Lazydog dbs.\n");
    printf("Options:\n");
    printf("\t-i <db1>\tSpecify the db you want to merge into the target db\n");
    printf("\t-h\tDisplay this help message and quit\n");
}

int main(int argc, char *argv[argc + 1]) {
    int opt;
    const char *input_db_path = NULL;
    const char *query = NULL;
    _Bool writeTsv = 0;

    while ((opt = getopt(argc, argv, "i:j:h")) != -1) {
        switch (opt) {
            case 'i':
                printf("Merging input db: %s\n", optarg);
                input_db_path = optarg;
                break;
            case 'j':
                printf("Will write tsv based on target-db and query %s\n", query);
                query = optarg;
                writeTsv = 1;
                break;
            case 'h':
                printHelp();
            return EXIT_SUCCESS;
            default:
                printf("Unknown option %c - skipping\n", opt);
        }
    }
    const int non_opt_args_given = argc - optind;
    if (non_opt_args_given != 1) {
        fprintf(stderr, "Please pass a single target db - (free args given %d)\n", non_opt_args_given);
        exit(EXIT_FAILURE);
    }
    const char *target_db_path = argv[optind++];

    int res = EXIT_FAILURE;
    if (writeTsv) {
        struct samples_vec *samps = get_samples(target_db_path, query);
        res = write_tsv_file(samps);
        FreeSamples(samps);
        free(samps);
    } else if (input_db_path == NULL) {
        // Basic transform output and leave it
        res = transform_db(target_db_path);
    } else {
        res = transform_db(input_db_path);
        if (!res) {
            res = merge_db(input_db_path, target_db_path);
        }
    }

    exit(res);
}
