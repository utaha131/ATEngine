float2 SampleDisk(float2 random) {
  float angle = 2.0f * PI * random.x;
  return float2(cos(angle), sin(angle)) * sqrt(random.y);
}

float3 SampleCosineHemisphere(float2 random, out float solid_angle_pdf) {
  float2 tangential = SampleDisk(random);
  float elevation = sqrt(saturate(1.0f - random.y));
  solid_angle_pdf = elevation / PI;
  return float3(tangential.xy, elevation);
}