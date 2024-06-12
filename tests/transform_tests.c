#include <check.h>
#include "../model/transform.h"
#include "../model/generator.h"
#include "../model/combiner.h"


START_TEST(test_transform_dft_real_naive)
{
    generator_info_t info = { 
        .sampling_frequency = 256.0
    };

    double freq1 = 12.0;
    double freq2 = 20.0;
    real_signal_t signal = generate_sine(info, 1.0, 1.0 / freq1, 0.0, 5.0);
    real_signal_t sine2 = generate_sine(info, 1.0, 1.0 / freq2, 0.0, 5.0);
    
    real_signal_free_values(&sine2);

    add_signal(&signal, &sine2);

    complex_signal_t dftSignal = transform_dft_real_naive(&signal);

    // Check if the DFT signal has the same number of samples as the input signal
    ck_assert_uint_eq(dftSignal.info.num_samples, signal.info.num_samples);
    // Check if the DFT signal has the same sampling frequency as the input signal
    ck_assert_double_eq(dftSignal.info.sampling_frequency, signal.info.sampling_frequency);
    // Check if the DFT signal doesn't have a null pValues pointer
    ck_assert_ptr_ne(dftSignal.pValues, 0);
    
}
END_TEST

Suite *my_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("TransformsSuite");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_transform_dft_real_naive);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = my_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed != 0);
}
