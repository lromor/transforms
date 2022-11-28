// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// For an overview of the formulas read the readme, section FFT!
// There are multiple versions of the FFT, we implement a useful one
// (non recursive).
#include "common.h"
#include <cassert>
#include <cmath>
#include <cstddef>
#include <math.h>
#include <complex>
#include <iostream>
#include <numbers>

// Compute the unsigned log2 of an unsigned value.
static inline size_t uilog2(size_t value) {
  size_t log2 = (value & (value - 1)) != 0;
  while (value >>= 1) ++log2;
  return log2;
}

// Naive approach to reverse the first "width"
// LSBs of an integer n.
template <class T>
T reverse_bits(T n, short width) {
  T output = T(0);
  for (int i = 0; i < width; ++i) {
    const short pos = width - i - 1;
    output |= ((n >> pos) & 1) << i;
  }
  return output;
}

UniqueBuffer<std::complex<float>> FastFourierTransform(const UniqueBuffer<float> &x) {
  const size_t num_samples = x.Size();
  assert((num_samples & (num_samples - 1)) == 0 && "Num samples not a power of 2!");

  // Let's compute log2 of the number of samples.
  const unsigned log2_num_samples = log2(num_samples);
  UniqueBuffer<std::complex<float>> output(num_samples);

  // We have to do a bit-reverse copy of the input in the output.
  // What does it mean? It means we have to take the index value starting from zero,
  // reverse its bits, and use that number as output index!
  // For instance, 1010 -> 0101.
  for (size_t i = 0; i < x.Size(); ++i) {
    const size_t ri = reverse_bits<size_t>(i, log2_num_samples);
    output[ri] = x[i];
  }

  using namespace std::complex_literals;

  // We can work just on the output vector.
  // The fft allows us to iterate logn times.
  // https://en.wikipedia.org/wiki/Cooley%E2%80%93Tukey_FFT_algorithm.
  // This is the iterative radix-2 FFT algorithm implemented using bit-reversal permutation.
  for (size_t s = 0; s < log2_num_samples; ++s) {
    // s-index represents size of the "fft leaf".
    // For instance, for 8 samples, the first iteration will take care of computing the sub-ffts
    // of size 2 and will generate then 8 intermediate values that in the next iteration will be used
    // to compute the sub-ffts of size 4! A good way of understand this is drawing the butterfly diagram.
    // https://www.youtube.com/watch?v=1mVbZLHLaf0&t=1810s.
    // That is why we can use directly the output vector to compute the final fft!
    const size_t m = 1 << (s + 1);

    // Weight to be multiplied for every fft.
    std::complex<float> w_m = std::polar<float>(1, - M_PI * 2 / m);

    // First iterate through every sub-fft. Index "k" is the starting
    // index of the output vector for the sub-fft m. For instance the
    // first iteration will be 0, 2, 4, the second will be 0, 4, 8, etc..
    for (size_t k = 0; k < num_samples; k += m) {
      std::complex<float> w = 1;
      for (size_t j = 0; j < (m / 2); ++j) {
        // Butterfly diagram "diagonal" edges, they are multiplied by w.
        const auto t = w * output[k + j + m / 2];

        // Butterfly diagram "direct" edges. Not multiplied by anything.
        const auto u = output[k + j];

        // Update the values for this couple of sub-ffts for
        // both the sub-coefficients.
        output[k + j] = u + t;
        output[k + j + m / 2] = u - t;

        // Prepare w for the next sub-coefficients!
        w = w * w_m;
      }
    }
  }
  return output;
}

UniqueBuffer<std::complex<float>> DiscreteFourierTransform(const UniqueBuffer<float> &x) {
  const size_t num_samples = x.Size();
  UniqueBuffer<std::complex<float>> output(num_samples);
  using namespace std::complex_literals;

  // Outer loop, i represents every "frequency" coefficient index.
  for (size_t k = 0; k < num_samples; ++k) {
    // https://en.wikipedia.org/wiki/Discrete_Fourier_transform#Definition, (Eq.1)
    for (size_t n = 0; n < num_samples; ++n) {
      output[k] += x[n] * (std::cos(static_cast<float>(M_PI * 2 * k * n / num_samples)) - std::complex<float>(0, std::sin(static_cast<float>(M_PI * 2 * k * n / num_samples))));
    }
  }
  return output;
}

int main (int argc, char *argv[]) {
  // Let's create a sample signal! We don't care about anything too complicated for now.
  // A simple trigonometric function, should suffice.
  // To simplify things, let's keep the sizes as a power of 2. I'ts not too relevant for the
  // standard DFT definition, but it is for the FFT we are gonna do after.
  const size_t kNumSamples = 1024;

  // Let's define the periodic interval T. We sample our function
  // from [0, T).
  const float kSamplingInterval = 8 * M_PI;

  // Allocate an empty buffer.
  UniqueBuffer<float> samples_space(kNumSamples);

  // Let's fill our data with sin samples. Note that this is just test data, you are free to fill it in
  // with any function!
  const float kSinFrequencyParam = 10;
  const float sampling_period = kSamplingInterval / kNumSamples;  // Time delta between each sample.
  const float sample_rate = 1.0 / sampling_period;
  for (size_t i = 0; i < samples_space.Size(); ++i) {
    const float t = sampling_period * i;
    samples_space[i] = std::sin(M_PI * 2 * kSinFrequencyParam * t);
  }

  UniqueBuffer<std::complex<float>> samples_frequency = FastFourierTransform(samples_space);
  for (size_t i = 0; i < samples_space.Size(); ++i) {
    std::cout << i * sample_rate / kNumSamples << " " << std::abs(samples_frequency[i]) << std::endl;
  }
}
