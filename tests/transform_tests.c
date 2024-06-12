#include <check.h>
#include <memory.h>
#include <string.h>
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

    real_signal_free_values(&signal);

}
END_TEST

START_TEST(test_transform_generate_matrix_walsh_hadamard_recursive)
{

    /*
        The Walsh-Hadamard matrix of order 1 is:
        1  1
        1 -1
    */
    double* pMatrix = transform_generate_matrix_walsh_hadamard_recursive(1);

    ck_assert_ptr_ne(pMatrix, 0);

    ck_assert_double_eq(pMatrix[0], 1.0);
    ck_assert_double_eq(pMatrix[1], 1.0);
    ck_assert_double_eq(pMatrix[2], 1.0);
    ck_assert_double_eq(pMatrix[3], -1.0);

    free(pMatrix);

}
END_TEST

START_TEST(test_transform_walsh_hadamard_real_naive)
{

    real_signal_t inputSignal = {
        .info = {
            .start_time = 0.0,
            .sampling_frequency = 256.0,
            .num_samples = 2
        }
    };
    real_signal_alloc_values(&inputSignal);
    inputSignal.pValues[0] = 1.0;
    inputSignal.pValues[1] = 2.0;

    walsh_hadamard_config_t config = {
        .m = 1
    };

    real_signal_t whSignal = transform_walsh_hadamard_real_naive(&inputSignal, &config);

    /*
        The normalized Walsh-Hadamard matrix of order 1 is:
        1/sqrt(2)  1/sqrt(2)
        1/sqrt(2)  -1/sqrt(2)

        The input signal is:
        1
        2

        The output signal is:
        3 / sqrt(2)
        -1 / sqrt(2)
    */

    ck_assert_uint_eq(whSignal.info.num_samples, 2);
    ck_assert_double_eq(whSignal.pValues[0], 3.0 / sqrt(2));
    ck_assert_double_eq(whSignal.pValues[1], -1.0 / sqrt(2));

    real_signal_free_values(&inputSignal);
    real_signal_free_values(&whSignal);

}
END_TEST

START_TEST(test_transform_walsh_hadamard_real_fast)
{
    real_signal_t inputSignal = {
        .info = {
            .start_time = 0.0,
            .sampling_frequency = 256.0,
            .num_samples = 2
        }
    };
    real_signal_alloc_values(&inputSignal);
    inputSignal.pValues[0] = 1.0;
    inputSignal.pValues[1] = 2.0;

    walsh_hadamard_config_t config = {
        .m = 1
    };

    real_signal_t whSignal = transform_walsh_hadamard_real_fast(&inputSignal, &config);

    /*
        The normalized Walsh-Hadamard matrix of order 1 is:
        1/sqrt(2)  1/sqrt(2)
        1/sqrt(2)  -1/sqrt(2)

        The input signal is:
        1
        2

        The output signal is:
        3 / sqrt(2)
        -1 / sqrt(2)
    */

    ck_assert_uint_eq(whSignal.info.num_samples, 2);
    ck_assert_double_eq(whSignal.pValues[0], 3.0 / sqrt(2));
    ck_assert_double_eq(whSignal.pValues[1], -1.0 / sqrt(2));

    real_signal_free_values(&inputSignal);
    real_signal_free_values(&whSignal);
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
    tcase_add_test(tc_core, test_transform_generate_matrix_walsh_hadamard_recursive);
    tcase_add_test(tc_core, test_transform_walsh_hadamard_real_naive);
    tcase_add_test(tc_core, test_transform_walsh_hadamard_real_fast);
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
