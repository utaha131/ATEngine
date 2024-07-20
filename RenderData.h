#ifndef _AT_RENDER_DATA_H_
#define _AT_RENDER_DATA_H_
#include <DirectXMath.h>
#include <vector>
#include <array>

namespace AT {

	struct LightProbe {
		DirectX::XMFLOAT4A Position;
	};

	struct ReflectionProbe {
		DirectX::XMFLOAT4A Position;
	};

	struct RenderObject {
		uint32_t MeshID;
		DirectX::XMFLOAT4X4 Transform;
	};

	enum LIGHT_TYPE {
		LIGHT_TYPE_NONE,
		LIGHT_TYPE_DIRECTIONAL,
		LIGHT_TYPE_POINT,
	};

	struct Light {
		DirectX::XMFLOAT4A PositionOrDirection;
		DirectX::XMFLOAT4A Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		DirectX::XMFLOAT4X4A LightMatrices[4];
		LIGHT_TYPE Type;
		float Intensity;
		float Radius;
	};

	class Mesh;
	class Material;
	struct RenderData {
		uint64_t FrameNumber;
		float FOV;
		DirectX::XMVECTOR CameraPosition;
		DirectX::XMMATRIX ProjectionMatrix;
		DirectX::XMMATRIX ViewMatrix;
		DirectX::XMMATRIX ViewProjectionMatrix;
		DirectX::XMMATRIX PreviousViewProjectionMatrix;
		DirectX::XMMATRIX InverseProjectionMatrix;
		DirectX::XMMATRIX InverseViewMatrix;
		DirectX::XMMATRIX InverseViewProjectionMatrix;
		std::optional<ReflectionProbe> ReflectionProbe;
		std::optional<LightProbe> LightProbe;
		std::vector<RenderObject> RenderObjects;
		std::vector<Mesh*> Meshes;
		std::vector<Material*> Materials;
		std::vector<Light> Lights;
	};
}
#endif