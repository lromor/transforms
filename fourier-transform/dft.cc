// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// For an overview of the formulas read the readme, section DFT!
#include "common.h"
#include <cassert>
#include <cmath>
#include <math.h>
#include <complex>
#include <iostream>

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
  assert((kNumSamples & (kNumSamples - 1)) == 0 && "Num samples not a power of 2!");

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

  // Nice! We have our function values stored in a vector.
  // Notice that the vector itself is not encoding anything about the spacing of the samples.
  // At this point all we care about is to perform the slow version of the DFT.
  // Note that the fourier transform output is in the general case, made of complex values!
  // Our output will be an array of complex values.
  // Allocate an empty buffer of complex values. Note that we have as many
  // coefficients as there are samples.
  // No need to pass any information about the sampling frequency! It's just a normalization constant.
  UniqueBuffer<std::complex<float>> samples_frequency = DiscreteFourierTransform(samples_space);

  for (size_t i = 0; i < samples_space.Size(); ++i) {
    //std::cout << i * sample_rate / kNumSamples << " " << std::abs(samples_frequency[i]) << std::endl;
    std::cout << i << " " << std::abs(samples_frequency[i]) << std::endl;
  }
}


// Questions:
// - What is the nyquist frequency?
// - What is the meaning of the frequencies higher than the nyqust frequency?
//   It's the max frequency at which point aliasing makes the coefficient correlate with lower and lower frequencies.
//   This is due to the exponent denominator that has a periodicity dependent on N.This is called aliasing of the DFT.
