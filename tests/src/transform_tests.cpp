#include <gtest/gtest.h>
#include "../../model/transform.h"
#include "../../model/generator.h"
#include "../../model/combiner.h"

TEST(Transforms, transform_dft_real_naive)
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

    EXPECT_EQ(dftSignal.info.num_samples, signal.info.num_samples);
    EXPECT_EQ(dftSignal.info.sampling_frequency, signal.info.sampling_frequency);
    //EXPECT_NE(dftSignal.pValues, nullptr);

}