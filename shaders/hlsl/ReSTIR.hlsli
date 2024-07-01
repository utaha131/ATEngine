#include "Random.hlsli"

template <typename Sample> struct Resivoir {
  Sample OutSample;
  float WeightSum;
  void AddSample(inout RNGSampler rng, Sample add_sample, float w) {
    WeightSum += w;
    if (rng.SampleUniform() < w / WeightSum) {
      OutSample = add_sample;
    }
  }
};
