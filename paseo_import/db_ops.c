#include "db_ops.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *build_path_arg(const char *path_indb) {
    char *path_specify = "file:";
    const size_t p1_size = strlen(path_specify);
    const size_t p2_size = strlen(path_indb);
    const size_t total_len = p1_size + p2_size + 1;
    char *path_arg = malloc(total_len);
    if (path_arg == NULL) {
        fprintf(stderr, "Malloc failed for %lu\n", total_len);
        exit(EXIT_FAILURE);
    }
    memcpy(path_arg, path_specify, p1_size);
    memcpy(path_arg + p1_size, path_indb, p2_size);
    path_arg[total_len - 1] = '\0';
    return path_arg;
}

//// Transforming a standard (export) sqlite db to lazydog-style db

static int apply_transform_steps(sqlite3 *db);

static int check_paseo_tables(sqlite3 *db);

static int add_meta_loading_info(sqlite3 *db);

int transform_db(const char *path_indb) {
    sqlite3 *db;

    if (access(path_indb, F_OK) == -1) {
        fprintf(stderr, "Can't find input db: %s\n", path_indb);
        return EXIT_FAILURE;
    }

    char *path_arg = build_path_arg(path_indb);
    int func_return_code = EXIT_FAILURE;

    const int open_ret_code = sqlite3_open(path_arg, &db);
    if (open_ret_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        goto clean_and_return;
    }
    const int steps_needed = check_paseo_tables(db);
    if (steps_needed == 2) {
        fprintf(stderr, "Failed table check\n");
        goto clean_and_return;
    }
    if (steps_needed) {
        func_return_code = apply_transform_steps(db);
    } else {
        printf("Skipping transformations for %s\n", path_indb);
        func_return_code = EXIT_SUCCESS;
    }
    if (func_return_code) {
        fprintf(stderr, "Failed table transform\n");
        goto clean_and_return;
    }
    func_return_code = add_meta_loading_info(db);

clean_and_return:
    sqlite3_close(db);
    free(path_arg);
    return func_return_code;
}

static const char *tracked_steps_ddl =
        "CREATE TABLE paseo_tracked_steps (source_id INT PRIMARY KEY, start_hour TEXT, steps_in_hour INT); ";

static const char *tracked_steps_sql =
        "INSERT INTO paseo_tracked_steps "
        "SELECT "
        "_id as source_id, "
        "datetime(substr(`date`, 1, 4) || '-' || substr(`date`, 5, 2) || '-' || substr(`date`, 7, 2) || 'T' || PRINTF('%02d',`hour`) || ':00') as `start_hour`, "
        "(endSteps - startSteps) as steps_in_hour "
        "FROM hours; ";

static const char *temp_hour_diffs_sql =
        "CREATE TEMPORARY TABLE temp_hour_diffs AS "
        "SELECT "
        "`_id`, "
        "(startSteps - LAG(startSteps, 1) OVER (ORDER BY `date`, `hour`)) AS step_difference "
        "FROM hours "
        "ORDER BY `date`, `hour`; ";

static const char *lost_steps_ddl =
        "CREATE TABLE paseo_lost_steps (source_id INTEGER PRIMARY KEY ASC, start_hour TEXT NOT NULL, found_steps INT, steps_in_hour INT, step_difference INT); ";

static const char *lost_steps_sql =
        "INSERT INTO paseo_lost_steps "
        "SELECT "
        "tab_hours._id as source_id, "
        "datetime(substr(tab_hours.date, 1, 4) || '-' || substr(tab_hours.date, 5, 2) || '-' || substr(tab_hours.date, 7, 2) || 'T' || PRINTF('%02d',tab_hours.hour) || ':00') as `start_hour`, "
        "tab_hours.startSteps as found_steps, "
        "(tab_hours.endSteps - tab_hours.startSteps) as steps_in_hour, "
        "tab_diff.step_difference "
        "FROM hours tab_hours "
        "INNER JOIN (SELECT "
        "`_id`, "
        "step_difference "
        "FROM temp.temp_hour_diffs "
        "WHERE step_difference < 0) tab_diff "
        "ON tab_hours._id = tab_diff._id "
        "ORDER BY tab_hours.date, tab_hours.hour; ";

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    // TODO: Is this code called? Can we use it?
    for (int i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

static int apply_transform_steps(sqlite3 *db) {
    char *zErrMsg = 0;
    int ret_code = sqlite3_exec(db, tracked_steps_ddl, callback, 0, &zErrMsg);
    if (ret_code != SQLITE_OK) {
        fprintf(stderr, "SQL error creating tracked steps table: %s\n", zErrMsg);
        goto fail_out;
    }
    fprintf(stdout, "Tracked steps table created\n");
    ret_code = sqlite3_exec(db, tracked_steps_sql, callback, 0, &zErrMsg);
    if (ret_code != SQLITE_OK) {
        fprintf(stderr, "SQL error transforming steps table: %s\n", zErrMsg);
        goto fail_out;
    }
    fprintf(stdout, "Tracked steps table transform applied and inserted\n");
    ret_code = sqlite3_exec(db, temp_hour_diffs_sql, callback, 0, &zErrMsg);
    if (ret_code != SQLITE_OK) {
        fprintf(stderr, "SQL error creating temp-hours-table: %s\n", zErrMsg);
        goto fail_out;
    }
    fprintf(stdout, "Temporary hour differences table created\n");
    ret_code = sqlite3_exec(db, lost_steps_ddl, callback, 0, &zErrMsg);
    if (ret_code != SQLITE_OK) {
        fprintf(stderr, "SQL error creating lost-steps-table: %s\n", zErrMsg);
        goto fail_out;
    }
    fprintf(stdout, "Created a table for \"lost\"steps between measurements\n");
    ret_code = sqlite3_exec(db, lost_steps_sql, callback, 0, &zErrMsg);
    if (ret_code != SQLITE_OK) {
        fprintf(stderr, "SQL error loading lost-steps-table: %s\n", zErrMsg);
        goto fail_out;
    }
    fprintf(stdout, "Loaded \"lost\"steps table\n");

    char *alter_statements_sql[] = {
        "ALTER TABLE `hours` RENAME TO paseo_hours;",
        "ALTER TABLE `android_metadata` RENAME TO paseo_android_metadata;",
        "ALTER TABLE `activityType` RENAME TO paseo_activity_type;",
    };
    for (int i = 0; i < 3; i++) {
        ret_code = sqlite3_exec(db, alter_statements_sql[i], callback, 0, &zErrMsg);
        if (ret_code != SQLITE_OK) {
            fprintf(stderr, "Error alterting table: %s\n", zErrMsg);
            goto fail_out;
        }
    }

    fprintf(stdout, "Renamed tables to paseo specific naming\n");
    return EXIT_SUCCESS;

fail_out:
    sqlite3_free(zErrMsg);
    return EXIT_FAILURE;
}

static int check_paseo_tables(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *query = "SELECT count(1) FROM sqlite_master WHERE type='table' and name like 'paseo%'; ";

    // Prepare the statement
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 2; // Can't continue anymore
    }

    int cnt = 0;
    // Execute the query
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        cnt = sqlite3_column_int(stmt, 0);
    }
    int rc;
    if ((rc = sqlite3_step(stmt)) != SQLITE_DONE) {
        fprintf(stderr, "Not finished after 1 step - code %d: %s\n", rc, sqlite3_errmsg(db));
        // Don't do anything for now
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
    if (cnt == 5) {
        return 0; // No need to transform
    }
    if (cnt == 0) {
        return 1; // Please transform the tables
    }
    fprintf(stderr, "Unexpected count %d of paseo tables\n", cnt);
    return 2; // Can't continue anymore
}

//// Merging

static int apply_merge_step(sqlite3 *db);

static int apply_copy_step(sqlite3 *db);

static int attach_db(const char *path_indb_1, sqlite3 *db) {
    char query[256];
    const int errc = snprintf(query, 256, "ATTACH DATABASE \'%s\' AS dba; ", path_indb_1);
    if (errc < 0) {
        fprintf(stderr, "Failed to create ATTACH-query for %s\n", path_indb_1);
        return EXIT_FAILURE;
    }

    char *zErrMsg = 0;
    const int ret_code = sqlite3_exec(db, query, callback, 0, &zErrMsg);
    if (ret_code != SQLITE_OK) {
        fprintf(stderr, "SQL error transforming steps table: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return EXIT_FAILURE;
    }
    printf("Attached table %s to target-db\n", path_indb_1);
    return EXIT_SUCCESS;
}

int merge_db(const char *path_indb_1, const char *path_indb_2) {
    sqlite3 *db;
    char *path_arg_2 = build_path_arg(path_indb_2);

    int func_ret_code = EXIT_FAILURE;

    func_ret_code = sqlite3_open(path_arg_2, &db);
    if (func_ret_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        goto clean_and_return;
    }
    printf("Opened target-db %s\n", path_arg_2);

    // Check if we need to create new tables or join the data
    const int table_info = check_paseo_tables(db);
    if (table_info == 2) {
        func_ret_code = EXIT_FAILURE;
        fprintf(stderr, "Failed table check\n");
        goto clean_and_return;
    }
    // Attach the 2nd table
    func_ret_code = attach_db(path_indb_1, db);
    if (func_ret_code) {
        goto clean_and_return;
    }

    if (table_info) {
        // copy tables
        func_ret_code = apply_copy_step(db);
    } else {
        // merge tables
        func_ret_code = apply_merge_step(db);
    }
    if (func_ret_code) {
        goto clean_and_return;
    }
    func_ret_code = add_meta_loading_info(db);

clean_and_return:
    sqlite3_close(db);
    free(path_arg_2);
    return func_ret_code;
}

static int apply_merge_step(sqlite3 *db) {
    char *merge_statements_sql[] = {
        "INSERT OR REPLACE INTO paseo_tracked_steps SELECT * FROM dba.paseo_tracked_steps; ",
        "INSERT OR REPLACE INTO paseo_lost_steps SELECT * FROM dba.paseo_lost_steps; ",
        "INSERT OR REPLACE INTO paseo_android_metadata SELECT * FROM dba.paseo_android_metadata; ",
        "INSERT OR REPLACE INTO paseo_activity_type SELECT * FROM dba.paseo_activity_type; ",
    };
    for (int i = 0; i < 4; i++) {
        char *zErrMsg = 0;
        const int ret_code = sqlite3_exec(db, merge_statements_sql[i], callback, 0, &zErrMsg);
        if (ret_code != SQLITE_OK) {
            fprintf(stderr, "SQL error transforming steps table: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return EXIT_FAILURE;
        }
        printf("Finished merge %d\n", i);
    }
    return EXIT_SUCCESS;
}

static int apply_copy_step(sqlite3 *db) {
    char *copy_statements_sql[] = {
        "CREATE TABLE paseo_tracked_steps AS SELECT * FROM dba.paseo_tracked_steps; ",
        "CREATE TABLE paseo_lost_steps AS SELECT * FROM dba.paseo_lost_steps; ",
        "CREATE TABLE paseo_android_metadata AS SELECT * FROM dba.paseo_android_metadata; ",
        "CREATE TABLE paseo_activity_type AS SELECT * FROM dba.paseo_activity_type; ",
    };
    for (int i = 0; i < 4; i++) {
        char *zErrMsg = 0;
        const int ret_code = sqlite3_exec(db, copy_statements_sql[i], callback, 0, &zErrMsg);
        if (ret_code != SQLITE_OK) {
            fprintf(stderr, "SQL error transforming steps table: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return EXIT_FAILURE;
        }
        printf("Finished copy %d\n", i);
    }
    return EXIT_SUCCESS;
}

// Adding loading info

static int add_meta_loading_info(sqlite3 *db) {
    char *meta_info_sql[] = {
        "CREATE TABLE IF NOT EXISTS tool_info ("
        "tool_name TEXT NOT NULL PRIMARY KEY, "
        "latest_version TEXT NOT NULL, "
        "creation_time TEXT NOT NULL, "
        "latest_import TEXT NOT NULL); ",
        "INSERT INTO tool_info (tool_name, latest_version, creation_time, latest_import) "
        "VALUES ('paseo-to-lazydog', '0.2.0', datetime('now'), datetime('now')) "
        "ON CONFLICT(tool_name) DO UPDATE SET latest_version = '0.2.0', latest_import = datetime('now'); ",
    };
    for (int i = 0; i < 2; i++) {
        char *zErrMsg = 0;
        const int ret_code = sqlite3_exec(db, meta_info_sql[i], callback, 0, &zErrMsg);
        if (ret_code != SQLITE_OK) {
            fprintf(stderr, "SQL error adding meta info: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return EXIT_FAILURE;
        }
        printf("Finished adding meta info %d\n", i);
    }
    return EXIT_SUCCESS;
}

//// Retrieve "generic" step data

// out1 and out2 must be able to hold 11 chars
int split_range_query(const char *in_range, char *restrict out1, char *restrict out2) {
    // Default str "1999-10-20:2002-02-21"
    if (strlen(in_range) != 21) {
        fprintf(stderr, "Input string too small: %s\n", in_range);
        return EXIT_FAILURE;
    }
    if (in_range[10] != ':') {
        fprintf(stderr, "Colon not found in the right spot in: %s\n", in_range);
        return EXIT_FAILURE;
    }
    const int dashes[] = {4, 7, 15, 18};
    for (int i = 0; i < 4; i++) {
        const int idx = dashes[i];
        if (in_range[idx] != '-') {
            fprintf(stderr, "Dashes not found in the right spot in: %s (%d)\n", in_range, idx);
            return EXIT_FAILURE;
        }
    }
    memcpy(out1, in_range, 10);
    out1[10] = '\0';
    memcpy(out2, in_range + 11, 10);
    out2[10] = '\0';

    return EXIT_SUCCESS;
}

static int add_samples(sqlite3 *db, struct samples_vec *samples, const char *query_range) {
    char gtq[11];
    char ltq[11];
    int rc = split_range_query(query_range, gtq, ltq);
    if (rc) {
        return EXIT_FAILURE; // Can't continue anymore
    }

    sqlite3_stmt *stmt;
    const char *query =
            "select steps_in_hour, unixepoch(start_hour) from paseo_tracked_steps where start_hour >= ? and start_hour < ?; ";

    // Prepare the statement
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return EXIT_FAILURE;
    }
    int retCode = EXIT_FAILURE;
    rc = sqlite3_bind_text(stmt, 1, gtq, -1, NULL);
    if (rc) {
        fprintf(stderr, "Failed to bind statement param 1: %s\n", sqlite3_errmsg(db));
        goto wind_down;
    }
    rc = sqlite3_bind_text(stmt, 2, ltq, -1, NULL);
    if (rc) {
        fprintf(stderr, "Failed to bind statement param 2: %s\n", sqlite3_errmsg(db));
        goto wind_down;
    }

    // Execute the query
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const int stepsCol = sqlite3_column_int(stmt, 0);
        const long timeCol = sqlite3_column_int64(stmt, 1);
        AddSample(samples, stepsCol, timeCol);
    }
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    } else {
        retCode = EXIT_SUCCESS;
    }

wind_down:
    // Finalize the statement
    sqlite3_finalize(stmt);
    return retCode; // All samples added
}

// Returns null in case it did not succeed
// Please free output-samples after use
struct samples_vec *get_samples(const char *path_db, const char *query_range) {
    struct samples_vec *samples = malloc(sizeof(struct samples_vec));
    if (samples == NULL) {
        fprintf(stderr, "Malloc failed for sample_vec\n");
        exit(EXIT_FAILURE);
    }
    NewSamples(samples, 1024);

    sqlite3 *db;
    char *path_arg = build_path_arg(path_db);
    const int err_code = sqlite3_open(path_arg, &db);
    if (err_code != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(err_code);
    }
    printf("Opened target-db %s\n", path_arg);
    add_samples(db, samples, query_range);

    free(path_arg);
    return samples;
}
