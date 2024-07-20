#ifndef _AT_SCENE_H_
#define _AT_SCENE_H_
#include <DirectXMath.h>
#include <memory>

#include "../Systems/RenderSystem/GPUResourceManager.h"
#include "../Systems/RenderSystem/MeshManager.h"
#include "../Systems/RenderSystem/MaterialManager.h"
#include "../ResourceManagers/ImageManager.h"
#include "../RenderData.h"

namespace AT {
	struct Camera {
		float VerticalFOV;
		DirectX::XMVECTOR Position;
		DirectX::XMVECTOR Rotation;
	};

	class Scene {
		friend class SceneManager;
	public:
		struct SceneNode {
			std::vector<uint32_t> Mesh_Ids;
			DirectX::XMMATRIX Transform;
			DirectX::XMMATRIX Inverse;
			std::vector<SceneNode*> Children;
			SceneNode* Parent;
		};

		Scene() {
			m_Lights.push_back(Light{
				.PositionOrDirection = { 0.0001f, 1.0f, 0.2f, 0.0f },
				.Type = LIGHT_TYPE_DIRECTIONAL,
				.Intensity = 30.0f,
			});
			m_Lights.push_back(Light{
				.PositionOrDirection = { -9.0f, 1.0f, 0.0f, 1.0f },
				.Type = LIGHT_TYPE_POINT,
				.Intensity = 30.0f,
				.Radius = 10.0f,
			});
		}

		std::vector<RenderObject> GenerateRenderObjects() {
			std::vector<RenderObject> render_objects;
			DirectX::XMMATRIX model = DirectX::XMMatrixSet(1.0f, 0.0f, 0.0f, 0.0f,
														   0.0f, 1.0f, 0.0f, 0.0f,
														   0.0f, 0.0f, 1.0f, 0.0f,
														   0.0f, 0.0f, 0.0f, 1.0f);
			ParseSceneToRenderObjects(m_SceneRoot, render_objects, model);
			return render_objects;
		}
		Camera Camera;

		void GenerateRenderData(RenderData& render_data) {
			render_data.Meshes = m_Meshes;
			render_data.Materials = m_Materials;
			render_data.Lights = m_Lights;
			render_data.ReflectionProbe = ReflectionProbe{};
			DirectX::XMStoreFloat4A(&render_data.ReflectionProbe->Position, Camera.Position);

			//Fill in Camera Position and Matrices.
			render_data.FOV = Camera.VerticalFOV;
			render_data.CameraPosition = this->Camera.Position;
			render_data.ProjectionMatrix = DirectX::XMMatrixPerspectiveFovRH(Camera.VerticalFOV, 1280.0f / 720.0f, 1000.0f, 1.0f);
			DirectX::XMVECTOR rotation_quaternion = DirectX::XMQuaternionRotationRollPitchYawFromVector(this->Camera.Rotation);
			DirectX::XMMATRIX rotation_matrix = DirectX::XMMatrixRotationQuaternion(rotation_quaternion);
			render_data.ViewMatrix = DirectX::XMMatrixLookAtRH(this->Camera.Position, DirectX::XMVectorAdd(this->Camera.Position, DirectX::XMVector3Transform(DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), rotation_matrix)), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
			render_data.ViewProjectionMatrix = render_data.ViewMatrix * render_data.ProjectionMatrix;
			render_data.InverseProjectionMatrix = DirectX::XMMatrixInverse(nullptr, render_data.ProjectionMatrix);
			render_data.InverseViewMatrix = DirectX::XMMatrixInverse(nullptr, render_data.ViewMatrix);
			render_data.InverseViewProjectionMatrix = DirectX::XMMatrixInverse(nullptr, render_data.ViewProjectionMatrix);

			//Parse Scene Graph.
				DirectX::XMMATRIX model = DirectX::XMMatrixSet(1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
			ParseSceneToRenderObjects(m_SceneRoot, render_data.RenderObjects, model);
		}
		
	protected:
		inline void ParseSceneToRenderObjects(Scene::SceneNode* node, std::vector<RenderObject>& render_objects, DirectX::XMMATRIX transform) {
			DirectX::XMMATRIX model = transform * node->Transform;

			for (uint32_t i = 0; i < node->Children.size(); ++i) {
				ParseSceneToRenderObjects(node->Children[i], render_objects, model);
			}

			for (uint32_t i = 0; i < node->Mesh_Ids.size(); ++i) {
				uint32_t id = node->Mesh_Ids[i];
				RenderObject render_object = {};
				render_object.MeshID = id;
				DirectX::XMStoreFloat4x4(&render_object.Transform, model);
				render_objects.emplace_back(render_object);
			}
		}
	//private:
	public:
		SceneNode* m_SceneRoot;
		std::vector<Mesh*> m_Meshes;
		std::vector<Material*> m_Materials;
		std::unordered_map<std::string, GPUTexturePtr> m_TextureCache;
		//Light Data.
	public:
		std::vector<Light> m_Lights;
		std::vector<ReflectionProbe> m_Probes;
		DirectX::XMMATRIX m_PreviousViewProjectionMatrix;
	};
	typedef Scene* SceneRef;
}
#endif