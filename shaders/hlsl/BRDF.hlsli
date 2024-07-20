static const float PI = 3.14159265359f;
static const float INVERSE_PI = 1.0f / 3.14159265359f;
static const float3 F0_DIELECTRICS = float3(0.04f, 0.04f, 0.04f);

struct BSDFParameters {
	float3 L;
	float3 N;
	float3 V;
	float3 H;
	float3 R;
	float NdotL;
	float NdotV;
	float LdotH;
	float NdotH;
	float VdotH;
	float4 DiffuseColor;
	float3 SpecularColor;
	float Metalness;
	float Roughness;
};

float3 Lambertian_Diffuse(float3 diffuse) {
	return diffuse * INVERSE_PI;
}

float3 Fresnel_SchlickApprox(float3 F0, float VdotH) {
	return F0 + (float3(1.0f, 1.0f, 1.0f) - F0) *  exp2((-5.55473f * VdotH - 6.98316f) * VdotH);
}

float Distribution_GGX(float roughness, float NdotH) {
	float a = roughness * roughness;
	float a2 = a * a;
	float denom = (PI * pow(pow(NdotH, 2) * (a2 - 1) + 1, 2)) + 0.0001f;
	return a2 / denom;
}

float Geometric(float roughness, float NdotV, float NdotL) {
	float k = roughness * roughness / 2.0f;//(roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
	float G1 = NdotV / ((NdotV) * (1 - k) + k + 0.0001f);
	float G2 = NdotL / ((NdotL) * (1 - k) + k) + 0.0001f;
	return G1 * G2;
}

float3 BRDF(BSDFParameters parameters) {
	float3 Diffuse = Lambertian_Diffuse(parameters.DiffuseColor.rgb); 
	float D = Distribution_GGX(parameters.Roughness, parameters.NdotH);
	float3 F = Fresnel_SchlickApprox(parameters.SpecularColor, parameters.VdotH);
	float G = Geometric(parameters.Roughness, parameters.NdotV, parameters.NdotL);
	float3 Specular = (D * F * G) / (4.0f * parameters.NdotL * parameters.NdotV + 0.0000001f);
	return Diffuse;// + Specular; 
}

float3 Diffuse(BSDFParameters parameters) {
	return Lambertian_Diffuse(parameters.DiffuseColor.rgb);
}