#ifndef _BXDF_HLSL_
#define _BXDF_HLSL_

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
//#include "BRDF.hlsli"
#include "Microfacet.hlsli"
#include "Sampling.hlsli"

struct BxDFSample {
    float3 f;
    float3 wi;
    float pdf;
    bool IsSpecular;
};

struct LambertianBRDF {
    float3 BaseColor;
    float3 F(in float3 wo, in float3 normal, in float3 wi) {
        return BaseColor / PI;
    }

    float PDF(in float3 wo, in float3 normal, in float3 wi) {
        return abs(dot(normal, wi)) / PI;
    }
    
    BxDFSample Sample_F(float2 random, float u, in float3 wo, in float3 normal) {
        BxDFSample bxdf_sample;
        bxdf_sample.wi = normalize(SampleCosineHemisphere(random, bxdf_sample.pdf));
        bxdf_sample.f =  F(wo, normal, bxdf_sample.wi);
        return bxdf_sample;
    }
};

struct UE4BRDF {
    float4 BaseColor;
    float Roughness;
    float Metalness;

    float3 Diffuse() {
        return BaseColor.rgb * (1.0f - Metalness) / PI * BaseColor.a;
    }

    float3 Specular(in float3 wo, in float3 normal, in float3 wi) {
        float3 H = normalize(wo + wi);
        float NdotH = abs(dot(normal, H));
        float d = D(Roughness, NdotH);

        float NdotL = abs(dot(normal, wi));
        float NdotV = abs(dot(normal, wo));
        float g = G(Roughness, NdotV, NdotL);
        
        float VdotH = abs(dot(wo, H));
        float3 F0 = lerp(0.04f, BaseColor, Metalness).rgb;
        float3 f = Fresnel(F0, VdotH);

        return (d * f * g) / ( 4.0f * NdotL * NdotV + 0.0000001f ) * BaseColor.a;
    }


    float3 F(in float3 wo, in float3 normal, in float3 wi) {
        return (Diffuse() + Specular(wo, normal, wi));
    }

    float PDF(in float3 wo, in float3 normal, in float3 wi, in float diffuse_ratio) {
        float diffuse_pdf = abs(dot(normal, wi)) / PI;

        float3 H = normalize(wo + wi);
        float NdotH = abs(dot(normal, H));
        float d = D(Roughness, NdotH);

        float specular_pdf = d * NdotH / (4.0f * abs(dot(wo, H)) + 0.0000001f);
        return diffuse_pdf * (diffuse_ratio) + specular_pdf * (1.0f - diffuse_ratio);
    }

    BxDFSample Sample_F(float2 random, in float3 wo, in float3 normal) {
        BxDFSample bxdf_sample;

        float diffuse_ratio = 0.5f * (1.0f - Metalness);
        if (random.x < diffuse_ratio) {
            float3 local_direction = normalize(SampleCosineHemisphere(random, bxdf_sample.pdf));
            bxdf_sample.wi = normalize( TangentToWorld(local_direction, normal) );
            bxdf_sample.IsSpecular = false;
        } else {
            bxdf_sample.wi = normalize(ImportanceSampleGGX(random, Roughness, wo, normal));
            bxdf_sample.IsSpecular = true;
        }
        bxdf_sample.f =  F(wo, normal, bxdf_sample.wi);
        bxdf_sample.pdf = PDF(wo, normal, bxdf_sample.wi, diffuse_ratio);
        return bxdf_sample;
    }
};
#endif