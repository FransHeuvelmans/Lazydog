#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <cmocka.h>

#include "samples.h"

static void use_samples(void **state) {
    struct samples_vec raw_samples;
    struct samples_vec *my_samples = &raw_samples;
    NewSamples(my_samples, 10);
    assert_int_equal(my_samples->sample_count, 0);
    assert_int_equal(my_samples->sample_max, 10);
    assert_int_equal(raw_samples.sample_max, 10);

    for (long i = 20; i < 35; i++) {
        AddSample(my_samples, (float) i, i + 10);
    }
    for (size_t i = 0; i < my_samples->sample_count; i++) {
        const int out = (int) my_samples->samples[i];
        assert_int_equal(i + 20, out);
        const long outl = my_samples->sample_timestamps[i];
        assert_int_equal((long) i + 30, outl);
    }
    assert_int_equal(my_samples->sample_count, 15);
    assert_int_equal(my_samples->sample_max, 20); // 10 * 2

    FreeSamples(my_samples);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(use_samples),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
