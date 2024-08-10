#ifndef DB_OPS_H
#define DB_OPS_H
#include "samples.h"

char* build_path_arg(const char *path_indb);

int transform_db(const char* path_indb);

int merge_db(const char *path_indb_1, const char *path_indb_2);

int split_range_query(const char* in_range, char* restrict out1, char* restrict out2);

struct samples_vec *get_samples(const char *path_db, const char* query_range);

#endif //DB_OPS_H
