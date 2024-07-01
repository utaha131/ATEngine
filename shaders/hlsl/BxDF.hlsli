//Template 
//struct BxDF {
//  float3 F() {
//
//  }
//
//  float PDF() {
//
//  }
//  BxDFSample Sample_F() {
//
//  }
//};
#include "BRDF.hlsli"
#include "Sampling.hlsli"

struct BxDFSample {
    float3 f;
    float3 wi;
    float pdf;
};

struct LambertianBRDF {
    float3 BaseColor;
    float3 F(in float3 wo, in float3 normal, in float3 wi) {
        return BaseColor / PI;
    }

    float PDF(in float3 wo, in float3 normal, in float3 wi) {
        return abs(dot(normal, wi)) / PI;
    }
    
    BxDFSample Sample_F(float2 random, in float3 wo, in float3 normal, out float3 wi) {
        BxDFSample bxdf_sample;
        bxdf_sample.wi = normalize(SampleCosineHemisphere(random, bxdf_sample.pdf));
        bxdf_sample.f =  F(wo, normal, wi);
        return bxdf_sample;
    }
};

/*
float3 UnrealEngineSpecular(float3 wo, float3 wi, float3 normal, float3 base_color, float roughness, float metalness) {
  float3 F0 = lerp(F0_DIELECTRICS, base_color, metalness).rgb;
  float3 H = normalize(wo + wi);
  float D = Distribution_GGX(roughness, abs(dot(normal, H)));
	float3 F = Fresnel_SchlickApprox(F0, abs(dot(wo, H)));
	float G = Geometric(roughness, abs(dot(normal, wo)), abs(dot(normal, wi)));
	return (D * F * G) / (4.0f * abs(dot(normal, wi)) * abs(dot(normal, wo)) + 0.0000001f);
}

float UnrealEngineSpecularPDF(float3 wo, float3 wi, float3 normal, float roughness) {
  float3 H = normalize(wo + wi);
  float D = Distribution_GGX(roughness, abs(dot(normal, H)));
	return D * abs(dot(normal, H)) / (4.0f * abs(dot(wo, H)));
}

float3 UnrealEngineBRDF(float3 wo, float3 wi, float3 normal, float3 base_color, float roughness, float metalness) {
  float3 diffuse = base_color.rgb * (1.0f - metalness) * LambertianF(wo, wi);
  float3 specular = UnrealEngineSpecular(wo, wi, normal, base_color, roughness, metalness);
  return diffuse + specular;
}

float UnrealEnginePDF(float3 wo, float3 wi, float3 normal, float roughness, float weight) {
  return LambertianPDF(normal, wi) * (1.0f - weight) + weight * UnrealEngineSpecularPDF(wo, wi, normal, roughness);
}
*/

struct UE4BRDF {
    float3 BaseColor;
    float Roughness;
    float Metalness;
    float3 F(in float3 wo, in float3 normal, in float3 wi) {
        float3 F0 = lerp(F0_DIELECTRICS, BaseColor, Metalness).rgb;
        float3 H = normalize(wo + wi);
        float D = Distribution_GGX(Roughness, abs(dot(normal, H)));
        float3 F = Fresnel_SchlickApprox(F0, abs(dot(wo, H)));
        float G = Geometric(Roughness, abs(dot(normal, wo)), abs(dot(normal, wi)));
	    float3 specular = (D * F * G) / (4.0f * abs(dot(normal, wi)) * abs(dot(normal, wo)) + 0.0000001f);
        float3 diffuse = BaseColor / PI;
        return diffuse + specular;
    }

    float PDF(in float3 wo, in float3 normal, in float3 wi) {
        float diffuse_pdf = abs(dot(normal, wi)) / PI;

        float3 H = normalize(wo + wi);
        float D = Distribution_GGX(Roughness, abs(dot(normal, H)));
        float specular_pdf = D * abs(dot(normal, H)) / (4.0f * abs(dot(wo, H)));
        return diffuse_pdf * (1.0f - Metalness) + specular_pdf * Metalness;
    }

    BxDFSample Sample_F(float2 random, in float3 wo, in float3 normal, out float3 wi) {
        BxDFSample bxdf_sample;
        bxdf_sample.wi = normalize(SampleCosineHemisphere(random, bxdf_sample.pdf));
        bxdf_sample.f =  F(wo, normal, bxdf_sample.wi);
        return bxdf_sample;
    }
};