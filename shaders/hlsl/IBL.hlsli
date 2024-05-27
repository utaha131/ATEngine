//Reference: Real Shading in Unreal Engine 4 Siggraph 2013 (https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf)
static const float PI = 3.14159265359f;

float G(float alpha, float dot) {
    float k = alpha / 2.0f;
    float denom = ((dot) * (1.0f - k) + k) + 0.00001f;
    return dot / denom;
}

float G_Smith(float roughness, float NoV, float NoL) {
    float alpha = roughness * roughness;
    return G(alpha, NoV) * G(alpha, NoL);
}

float RadicalInverse_VdC(uint bits) {
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N) {
	return float2(float(i)/float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX( float2 Xi, float Roughness, float3 N ) {
  float a = Roughness * Roughness;
  float Phi = 2 * PI * Xi.x;
  float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
  float SinTheta = sqrt( 1 - CosTheta * CosTheta );
  float3 H;
  H.x = SinTheta * cos( Phi );
  H.y = SinTheta * sin( Phi );
  H.z = CosTheta;
  float3 UpVector = abs(N.z) < 0.999 ? float3(0,0,1) : float3(1,0,0);
  float3 TangentX = normalize( cross( UpVector, N ) );
  float3 TangentY = cross( N, TangentX );
  // Tangent to world space
  return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 PrefilterEnvMap(float Roughness, float3 R, TextureCube cube_map, SamplerState cube_map_sampler, uint mip_slice) {
  float3 N = R;
  float3 V = R;
  float3 PrefilteredColor = 0;
  const uint NumSamples = 64;
  float TotalWeight = 0.0f;
  for( uint i = 0; i < NumSamples; i++ ) {
    float2 Xi = Hammersley(i, NumSamples);
    float3 H = ImportanceSampleGGX(Xi, Roughness, N);
    float3 L = 2 * dot(V, H) * H - V;
    float NoL = saturate(dot( N, L ));
    if( NoL > 0 ) {
      PrefilteredColor += cube_map.SampleLevel(cube_map_sampler, L, mip_slice).rgb * NoL;
      TotalWeight += NoL;
    }
  }
  return PrefilteredColor / TotalWeight;
}

float2 IntegrateBRDF(float Roughness, float NoV) {
  float3 V;
  V.x = sqrt( 1.0f - NoV * NoV ); // sin
  V.y = 0;
  V.z = NoV; // cos
  float A = 0;
  float B = 0;
  const uint NumSamples = 64;
  
  float3 N = float3(0.0f, 0.0f, 1.0f);

  for( uint i = 0; i < NumSamples; i++ ) {
    float2 Xi = Hammersley(i, NumSamples);
    float3 H = ImportanceSampleGGX(Xi, Roughness, N);
    float3 L = 2 * dot(V, H) * H - V;
    float NoL = saturate(L.z);
    float NoH = saturate(H.z);
    float VoH = saturate(dot( V, H ));
    if(NoL > 0) {
      float G = G_Smith(Roughness, NoV, NoL);
      float G_Vis = G * VoH / (NoH * NoV);
      float Fc = pow(1 - VoH, 5);
      A += (1 - Fc) * G_Vis;
      B += Fc * G_Vis;
    }
  }
  return float2(A, B) / NumSamples;
}

float3 ApproximateSpecularIBL(float3 specular_color, float3 prefiltered_color, float2 env_brdf) {
  return prefiltered_color * (specular_color * env_brdf.x + env_brdf.y);
}