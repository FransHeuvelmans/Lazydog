#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>
#include "tsv.h"

/* A test case that does nothing and succeeds. */
static void test_write_samples(void **state) {
    struct samples_vec raw_samples;
    struct samples_vec *my_samples = &raw_samples;
    NewSamples(my_samples, 128);

    for (int i = 50; i < 75; i++) {
        AddSample(my_samples, (float) i, i * 10);
    }
    write_tsv_file(my_samples);
    FreeSamples(my_samples);
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_write_samples),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
