#include "Random.hlsli"
#include "Lights.hlsli"
#include "PathTracingHelper.hlsli"

#define ZERO_GUARD(x) max(0.0001f, x)

template <typename Sample> struct Reservoir {
  Sample OutSample;
  float WeightSum;
  float M;
  float W;

  void AddSample(inout RNGSampler rng, in Sample add_sample, float w) {
    M += 1.0f;
    WeightSum += w;
    if (rng.SampleUniform() < w / ZERO_GUARD(WeightSum)) {
      OutSample = add_sample;
    }
  }
};

typedef uint ReSTIR_DI_Sample;

template<typename BxDF> float ReSTIR_DI_TargetFunction(in LightSample light_sample, in SurfaceInteraction<BxDF> surface_interaction) {
  return length(surface_interaction.brdf.F(surface_interaction.wo, surface_interaction.Normal, light_sample.wi) * light_sample.L * 10.0f * abs(dot(light_sample.wi, surface_interaction.Normal)));
}


template <typename Sample> Reservoir<Sample> CombineReservoirs(inout RNGSampler rng, in Reservoir<Sample> r1, in float p_hat1, in Reservoir<Sample> r2, in float p_hat2) {
  Reservoir<Sample> s;
  //s.OutSample = 0;
  s.WeightSum = 0.0f;
  s.M = 0.0f;
  s.W = 0.0f;

  s.AddSample(rng, r1.OutSample, p_hat1 * r1.W * r1.M);
  s.AddSample(rng, r2.OutSample, p_hat2 * r2.W * r2.M);

  s.M = r1.M + r2.M;
  return s;
}

struct ReSTIR_GI_Sample {
  float3 X_s, Normal_s;
  float3 L_o;
  float3 Random;
};

float CalculateJacobian(float3 r, float3 q, Reservoir<ReSTIR_GI_Sample> temporal_reservoir) {
  float3 phi_r = r - temporal_reservoir.OutSample.X_s;
  float phi_r_length = length(phi_r);
  float3 phi_q =  q - temporal_reservoir.OutSample.X_s;
  float phi_q_length = length(phi_q);

  float cos_phi_r = abs(dot(normalize(temporal_reservoir.OutSample.Normal_s), phi_r / phi_r_length));
  float cos_phi_q = abs(dot(normalize(temporal_reservoir.OutSample.Normal_s), phi_q / phi_q_length));

  float jacobian = (cos_phi_r * phi_q_length * phi_q_length) / (cos_phi_q * phi_r_length * phi_r_length + 0.0001f);

  if (isnan(jacobian) || isinf(jacobian)) {
    jacobian = 0.0f;
  }

  return jacobian;
}

bool ValidateJacobian(inout float jacobian) {
    if (jacobian > 10.0f || jacobian < 1.0f / 10.0f) {
        return false;
    }

    jacobian = clamp(jacobian, 1.0f / 3.0f, 3.0f);
    return true;
}

template<typename Sample> bool IsValidReservoir(Reservoir<Sample> reservoir) {
  return (reservoir.M > 0.0f);
}