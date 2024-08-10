#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include "db_ops.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

static void copy_file(const char *source, const char *destination) {
    struct stat stat_buf;
    char buffer[1024];

    // Open the source file for reading
    const int src_fd = open(source, O_RDONLY);
    if (src_fd == -1) {
        perror("Error opening source file");
        return;
    }

    // Get the size of the source file
    if (fstat(src_fd, &stat_buf) == -1) {
        perror("Error getting file size");
        close(src_fd);
        return;
    }

    // Open the destination file for writing
    const int dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        perror("Error opening destination file");
        close(src_fd);
        return;
    }

    ssize_t bytes_read;
    // Copy the contents of the source file to the destination file
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        const ssize_t bytes_written = write(dest_fd, buffer, (size_t) bytes_read);
        if (bytes_written == -1) {
            perror("Error writing to destination file");
            close(src_fd);
            close(dest_fd);
            return;
        }
    }
    if (bytes_read == -1) {
        perror("Error reading from source file");
    }
    printf("Copied file %s - to - %s\n", source, destination);

    close(src_fd);
    close(dest_fd);
}

static void remove_file(const char *fpath) {
    printf("Deleting file %s\n", fpath);
    if (remove(fpath) != 0) {
        // Check if the error is because the file does not exist
        if (errno != ENOENT) {
            fprintf(stderr, "Failed to delete file: %s - %d \n", fpath, errno);
            assert_true(0);
        }
    }
}


static void test_bad_path(void **state) {
    char *bad_path = "/path/to/no/where";
    int err = transform_db(bad_path);
    assert_int_equal(err, EXIT_FAILURE);
}

static void test_good_path(void **state) {
    const char *gd_path = "../paseo_examples/paseoDB20240722-160856.db";
    remove_file(gd_path);
    const char *orig_path = "../paseo_examples/orig/paseoDB20240722-160856.db";
    copy_file(orig_path, gd_path);
    int err = transform_db(gd_path);
    assert_int_equal(err, EXIT_SUCCESS);
    err = transform_db(gd_path);
    assert_int_equal(err, EXIT_SUCCESS);

    char *qrange = "2024-04-01:2024-05-01";
    struct samples_vec *samples_out = get_samples(gd_path, qrange);
    assert_true(samples_out->sample_count > 0);
    FreeSamples(samples_out);
    free(samples_out);
}

static void test_other_good_path(void **state) {
    const char *gd_path = "../paseo_examples/paseoDB20231011-205117.db";
    remove_file(gd_path);
    const char *orig_path = "../paseo_examples/orig/paseoDB20231011-205117.db";
    copy_file(orig_path, gd_path);
    int err = transform_db(gd_path);
    assert_int_equal(err, EXIT_SUCCESS);
    err = transform_db(gd_path);
    assert_int_equal(err, EXIT_SUCCESS);

    char *qrange = "2023-08-13:2023-09-01";
    struct samples_vec *samples_out = get_samples(gd_path, qrange);
    assert_true(samples_out->sample_count > 0);
    FreeSamples(samples_out);
    free(samples_out);
}

static void test_merge_empty_path(void **state) {
    const char *gd_path = "../paseo_examples/paseoDB20240722-160856.db";
    remove_file(gd_path);
    const char *orig_path = "../paseo_examples/orig/paseoDB20240722-160856.db";
    copy_file(orig_path, gd_path);
    int err = transform_db(gd_path);
    assert_int_equal(err, EXIT_SUCCESS);

    const char *empty_path = "../paseo_examples/new_mrg.db";
    remove_file(empty_path);

    err = merge_db(gd_path, empty_path);
    assert_int_equal(err, EXIT_SUCCESS);
}

static void test_merge_2_db(void **state) {
    const char *gd_path_new = "../paseo_examples/paseoDB20240722-160856.db";
    remove_file(gd_path_new);
    const char *orig_path = "../paseo_examples/orig/paseoDB20240722-160856.db";
    copy_file(orig_path, gd_path_new);
    int err = transform_db(gd_path_new);
    assert_int_equal(err, EXIT_SUCCESS);

    const char *gd_path_old = "../paseo_examples/paseoDB20231011-205117.db";
    remove_file(gd_path_old);
    const char *orig_path_old = "../paseo_examples/orig/paseoDB20231011-205117.db";
    copy_file(orig_path_old, gd_path_old);
    err = transform_db(gd_path_old);
    assert_int_equal(err, EXIT_SUCCESS);

    err = merge_db(gd_path_new, gd_path_old);
    assert_int_equal(err, EXIT_SUCCESS);
}

static void test_some_range_queries(void **state) {
    const char *in1 = "1999-10-20:2002-02-21";
    const char *check1 = "1999-10-20";
    const char *check2 = "2002-02-21";

    char out1[11];
    char out2[11];
    int err = split_range_query(in1, out1, out2);
    assert_int_equal(err, EXIT_SUCCESS);
    assert_string_equal(check1, out1);
    assert_string_equal(check2, out2);

    const char *in2 = "2020-01-28:2060-11-03";
    const char *check21 = "2020-01-28";
    const char *check22 = "2060-11-03";
    err = split_range_query(in2, out1, out2);
    assert_int_equal(err, EXIT_SUCCESS);
    assert_string_equal(check21, out1);
    assert_string_equal(check22, out2);

    const char *in3 = "2020-01-28+2060-11-03";
    err = split_range_query(in3, out1, out2);
    assert_int_equal(err, EXIT_FAILURE);

    const char *in4 = "2020-01-28:2060-11+03";
    err = split_range_query(in4, out1, out2);
    assert_int_equal(err, EXIT_FAILURE);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_bad_path),
        cmocka_unit_test(test_good_path),
        cmocka_unit_test(test_other_good_path),
        cmocka_unit_test(test_merge_empty_path),
        cmocka_unit_test(test_merge_2_db),
        cmocka_unit_test(test_some_range_queries),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
