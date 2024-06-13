#include <check.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
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
    ck_assert_double_eq_tol(dftSignal.info.sampling_frequency, signal.info.sampling_frequency, 1e-10);
    // Check if the DFT signal doesn't have a null pValues pointer
    ck_assert_ptr_ne(dftSignal.pValues, 0);

    real_signal_free_values(&signal);
    complex_signal_free_values(&dftSignal);

}
END_TEST

START_TEST(test_transform_dft_real_fast_p2) {
    generator_info_t info = { 
        .sampling_frequency = 8.0
    };

    double freq1 = 2.0;
    double freq2 = 3.0;
    real_signal_t signal = generate_sine(info, 1.0, 1.0 / freq1, 0.0, 8.0);
    real_signal_t sine2 = generate_sine(info, 1.0, 1.0 / freq2, 0.0, 8.0);

    add_signal(&signal, &sine2);
    real_signal_free_values(&sine2);

    complex_signal_t dftSignal = transform_dft_real_fast_p2(&signal);

    // Check if the DFT signal has the same number of samples as the input signal
    ck_assert_uint_eq(dftSignal.info.num_samples, signal.info.num_samples);
    // Check if the DFT signal has the same sampling frequency as the input signal
    ck_assert_double_eq_tol(dftSignal.info.sampling_frequency, signal.info.sampling_frequency, 1e-10);
    // Check if the DFT signal doesn't have a null pValues pointer
    ck_assert_ptr_ne(dftSignal.pValues, 0);

    real_signal_free_values(&signal);
    complex_signal_free_values(&dftSignal);

}

#define CUSTOM_DEBUG

void debug_test() {
    real_signal_t inputSignal = {
        .info = {
            .start_time = 0.0,
            .sampling_frequency = 256.0,
            .num_samples = 4
        }
    };
    real_signal_alloc_values(&inputSignal);
    inputSignal.pValues[0] = 1.0;
    inputSignal.pValues[1] = 2.0;
    inputSignal.pValues[2] = 3.0;
    inputSignal.pValues[3] = 4.0;

    walsh_hadamard_config_t config = {
        .m = 2
    };

    real_signal_t whSignalNaive = transform_walsh_hadamard_real_naive(&inputSignal, &config);
    real_signal_t whSignalFast = transform_walsh_hadamard_real_fast(&inputSignal, &config);

    assert(whSignalNaive.info.num_samples == whSignalFast.info.num_samples);

    for (uint64_t i = 0; i < whSignalNaive.info.num_samples; i++) {
        double naiveValue = whSignalNaive.pValues[i];
        double fastValue = whSignalFast.pValues[i];

        fprintf(stdout, "[dbg] i = %lu, naiveValue = %lf, fastValue = %lf\n", i, naiveValue, fastValue);

        //assert(fabs(whSignalNaive.pValues[i] - whSignalFast.pValues[i]) < 1e-10);
    }

    real_signal_free_values(&inputSignal);
    real_signal_free_values(&whSignalNaive);
    real_signal_free_values(&whSignalFast);
}

void debug_test_ccc() {
    generator_info_t info = { 
        .sampling_frequency = 8.0
    };

    double freq1 = 2.0;
    double freq2 = 3.0;
    real_signal_t signal = generate_sine(info, 1.0, 1.0 / freq1, 0.0, 8.0);
    real_signal_t sine2 = generate_sine(info, 1.0, 1.0 / freq2, 0.0, 8.0);

    add_signal(&signal, &sine2);
    real_signal_free_values(&sine2);

    complex_signal_t dftNaive = transform_dft_real_naive(&signal);
    complex_signal_t dftFast = transform_dft_real_fast_p2(&signal);

    assert(dftNaive.info.num_samples == dftFast.info.num_samples);
    assert(fabs(dftNaive.info.sampling_frequency - dftFast.info.sampling_frequency) < 1e-10);

    for (uint64_t i = 0; i < dftNaive.info.num_samples; i++) {

        double naiveRe = creal(dftNaive.pValues[i]);
        double naiveIm = cimag(dftNaive.pValues[i]);
        double fastRe = creal(dftFast.pValues[i]);
        double fastIm = cimag(dftFast.pValues[i]);

        fprintf(stdout, "[dbg] i = %lu, naiveRe = %lf, fastRe = %lf, naiveIm = %lf, fastIm = %lf\n", i, naiveRe, fastRe, naiveIm, fastIm);

        // assert(fabs(naiveRe - fastRe) < 1e-5);
        // assert(fabs(naiveIm - fastIm) < 1e-5);
    }

    real_signal_free_values(&signal);
    complex_signal_free_values(&dftNaive);
    complex_signal_free_values(&dftFast);
}

START_TEST(test_check_dft_methods_equivalence) {
    generator_info_t info = { 
        .sampling_frequency = 8.0
    };

    double freq1 = 2.0;
    double freq2 = 3.0;
    real_signal_t signal = generate_sine(info, 1.0, 1.0 / freq1, 0.0, 8.0);
    real_signal_t sine2 = generate_sine(info, 1.0, 1.0 / freq2, 0.0, 8.0);

    add_signal(&signal, &sine2);
    real_signal_free_values(&sine2);

    complex_signal_t dftNaive = transform_dft_real_naive(&signal);
    complex_signal_t dftFast = transform_dft_real_fast_p2(&signal);

    ck_assert_uint_eq(dftNaive.info.num_samples, dftFast.info.num_samples);
    ck_assert_double_eq_tol(dftNaive.info.sampling_frequency, dftFast.info.sampling_frequency, 1e-10);

    for (uint64_t i = 0; i < dftNaive.info.num_samples; i++) {
        // print i
        fprintf(stdout, "[dbg] i = %lu\n", i);
        ck_assert_double_eq_tol(creal(dftNaive.pValues[i]), creal(dftFast.pValues[i]), 1e-5);
        ck_assert_double_eq_tol(cimag(dftNaive.pValues[i]), cimag(dftFast.pValues[i]), 1e-5);
    }

    real_signal_free_values(&signal);
    complex_signal_free_values(&dftNaive);
    complex_signal_free_values(&dftFast);
}

START_TEST(test_transform_generate_matrix_walsh_hadamard_recursive_1)
{

    /*
        The Walsh-Hadamard matrix of order 1 is:
        1  1
        1 -1
    */
    double* pMatrix = transform_generate_matrix_walsh_hadamard_recursive(1);

    ck_assert_ptr_ne(pMatrix, 0);

    ck_assert_double_eq_tol(pMatrix[0], 1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[1], 1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[2], 1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[3], -1.0, 1e-10);

    free(pMatrix);

}
END_TEST

START_TEST(test_transform_generate_matrix_walsh_hadamard_recursive_2)
{

    /*
        The Walsh-Hadamard matrix of order 2 is:
        1  1  1  1
        1 -1  1 -1
        1  1 -1 -1
        1 -1 -1  1
    */
    double* pMatrix = transform_generate_matrix_walsh_hadamard_recursive(2);

    ck_assert_ptr_ne(pMatrix, 0);

    ck_assert_double_eq_tol(pMatrix[0], 1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[1], 1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[2], 1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[3], 1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[4], 1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[5], -1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[6], 1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[7], -1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[8], 1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[9], 1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[10], -1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[11], -1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[12], 1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[13], -1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[14], -1.0, 1e-10);
    ck_assert_double_eq_tol(pMatrix[15], 1.0, 1e-10);

    free(pMatrix);

}

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
    ck_assert_double_eq_tol(whSignal.pValues[0], 3.0 / sqrt(2), 1e-10);
    ck_assert_double_eq_tol(whSignal.pValues[1], -1.0 / sqrt(2), 1e-10);

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
    ck_assert_double_eq_tol(whSignal.pValues[0], 3.0 / sqrt(2), 1e-10);
    ck_assert_double_eq_tol(whSignal.pValues[1], -1.0 / sqrt(2), 1e-10);

    real_signal_free_values(&inputSignal);
    real_signal_free_values(&whSignal);
}
END_TEST

START_TEST(test_check_walsh_hadamard_methods_equivalence) {
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

    real_signal_t whSignalNaive = transform_walsh_hadamard_real_naive(&inputSignal, &config);
    real_signal_t whSignalFast = transform_walsh_hadamard_real_fast(&inputSignal, &config);

    ck_assert_uint_eq(whSignalNaive.info.num_samples, whSignalFast.info.num_samples);

    for (uint64_t i = 0; i < whSignalNaive.info.num_samples; i++) {
        ck_assert_double_eq_tol(whSignalNaive.pValues[i], whSignalFast.pValues[i], 1e-10);
    }

    real_signal_free_values(&inputSignal);
    real_signal_free_values(&whSignalNaive);
    real_signal_free_values(&whSignalFast);
}

Suite *my_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("TransformsSuite");

    /* Core test case */
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_transform_dft_real_naive);
    tcase_add_test(tc_core, test_transform_dft_real_fast_p2);
    //tcase_add_test(tc_core, test_check_dft_methods_equivalence);

    tcase_add_test(tc_core, test_transform_generate_matrix_walsh_hadamard_recursive_1);
    tcase_add_test(tc_core, test_transform_generate_matrix_walsh_hadamard_recursive_2);
    tcase_add_test(tc_core, test_transform_walsh_hadamard_real_naive);
    tcase_add_test(tc_core, test_transform_walsh_hadamard_real_fast);
    //tcase_add_test(tc_core, test_check_walsh_hadamard_methods_equivalence);
    
    
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
#if defined(CUSTOM_DEBUG)
    debug_test(); return 0;
#endif

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
