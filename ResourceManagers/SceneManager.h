#ifndef _AT_RESOURCE_MANAGER_SCENE_MANAGER_H_
#define _AT_RESOURCE_MANAGER_SCENE_MANAGER_H_
#include <unordered_map>
#include <memory>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "../Scene/Scene.h"
#include "../Systems/JobSystem/JobSystem.h"

namespace AT {
	class SceneManager {
	public:
		SceneManager(MeshManager& mesh_manager, MaterialManager& material_manager) :
			m_MeshManager(mesh_manager),
			m_MaterialManager(material_manager)
		{

		}

		SceneRef LoadScene(const std::string& path) {
			if (m_SceneCache.find(path) != m_SceneCache.end()) {
				return m_SceneCache[path];
			}
			else {
				Assimp::Importer importer;
				const aiScene* ai_scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes);
				Scene* new_scene = new Scene();

				AT::JobSystem::JobCounter* mesh_job_counter = nullptr;
				JobSystem::Execute([=](uint32_t thread_ID) {
					ProcessAssimpSceneMeshes(ai_scene, new_scene->m_Meshes);
				}, &mesh_job_counter);

				AT::JobSystem::JobCounter* material_job_counter = nullptr;
				JobSystem::Execute([=](uint32_t thread_ID) {
					ProcessAssimpSceneMaterials(ai_scene, new_scene->m_Materials);
				}, &material_job_counter);

				ParseAssimpSceneTree(ai_scene->mRootNode, new_scene->m_SceneRoot, nullptr);
				JobSystem::WaitForCounter(mesh_job_counter, 0);
				delete mesh_job_counter;
				JobSystem::WaitForCounter(material_job_counter, 0);
				delete material_job_counter;

				m_SceneCache[path] = new_scene;
				return new_scene;
			}
		}
	protected:
		inline void ProcessAssimpSceneMaterials(const aiScene* ai_scene, std::vector<Material*>& materials) {
			materials = std::vector<Material*> (ai_scene->mNumMaterials);

			for (uint32_t i = 0; i < ai_scene->mNumMaterials; ++i) {
				aiMaterial* assimp_material = ai_scene->mMaterials[i];
				aiString path;
				aiColor3D color;
				assimp_material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
				float metallic;
				assimp_material->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
				float roughness;
				assimp_material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);

				DirectX::XMFLOAT3 Base_Color_Factor = { color.r, color.g, color.b };
				std::string Base_Color_Path = "", Normal_Path = "", Roughness_Metalness_Path = "";

				std::string prefix = "./Assets/Sponza/";

				if (assimp_material->GetTextureCount(aiTextureType_BASE_COLOR) != 0) {
					assimp_material->GetTexture(aiTextureType_BASE_COLOR, 0, &path);
					Base_Color_Path = prefix + std::string(path.C_Str());
				}

				if (assimp_material->GetTextureCount(aiTextureType_NORMALS) != 0) {
					assimp_material->GetTexture(aiTextureType_NORMALS, 0, &path);
					Normal_Path = prefix + std::string(path.C_Str());
				}

				if (assimp_material->GetTextureCount(aiTextureType_METALNESS) != 0) {
					assimp_material->GetTexture(aiTextureType_METALNESS, 0, &path);
					Roughness_Metalness_Path = prefix + std::string(path.C_Str());
				}

				materials[i] = m_MaterialManager.CreateMaterial("Sponza" + std::to_string(i), Base_Color_Factor, DirectX::XMFLOAT3{ 1.0f, roughness, metallic }, Base_Color_Path, Normal_Path, Roughness_Metalness_Path);
			}
		}
		inline void ProcessAssimpSceneMeshes(const aiScene* ai_scene, std::vector<Mesh*>& meshes) {
			meshes = std::vector<Mesh*>(ai_scene->mNumMeshes);

			for (uint32_t i = 0; i < ai_scene->mNumMeshes; ++i) {
				aiMesh* assimp_mesh = ai_scene->mMeshes[i];

				std::vector<AT::VertexFormat::Vertex> vertices = std::vector<AT::VertexFormat::Vertex>(assimp_mesh->mNumVertices);
				std::vector<AT::VertexFormat::Index> indices = std::vector<AT::VertexFormat::Index>();


				for (uint32_t j = 0; j < assimp_mesh->mNumVertices; ++j) {
					AT::VertexFormat::Vertex vertex = {};
					vertex.Position = DirectX::XMFLOAT3(assimp_mesh->mVertices[j].x, assimp_mesh->mVertices[j].y, assimp_mesh->mVertices[j].z);
					vertex.Normal = DirectX::XMFLOAT3(assimp_mesh->mNormals[j].x, assimp_mesh->mNormals[j].y, assimp_mesh->mNormals[j].z);
					if (assimp_mesh->HasTextureCoords(0)) {
						vertex.Tex_Coord = DirectX::XMFLOAT2(assimp_mesh->mTextureCoords[0][j].x, 1.0f - assimp_mesh->mTextureCoords[0][j].y);
					}
					else {
						vertex.Tex_Coord = DirectX::XMFLOAT2(0.0f, 0.0f);
					}
					vertex.Tangent = DirectX::XMFLOAT3(assimp_mesh->mTangents[j].x, assimp_mesh->mTangents[j].y, assimp_mesh->mTangents[j].z);
					vertex.BitTangent = DirectX::XMFLOAT3(assimp_mesh->mBitangents[j].x, assimp_mesh->mBitangents[j].y, assimp_mesh->mBitangents[j].z);
					vertices[j] = vertex;
				}

				for (uint32_t j = 0; j < assimp_mesh->mNumFaces; ++j) {
					aiFace& face = assimp_mesh->mFaces[j];
					assert(face.mNumIndices == 3);
					indices.push_back(static_cast<AT::VertexFormat::Index>(face.mIndices[0]));
					indices.push_back(static_cast<AT::VertexFormat::Index>(face.mIndices[1]));
					indices.push_back(static_cast<AT::VertexFormat::Index>(face.mIndices[2]));
				}
				meshes[i] = m_MeshManager.CreateMesh("Mesh" + std::to_string(i), vertices, indices, assimp_mesh->mMaterialIndex);
			}
		}
		inline void ParseAssimpSceneTree(aiNode* node, Scene::SceneNode*& scene_node, Scene::SceneNode* parent) {
			scene_node = new Scene::SceneNode();
			scene_node->Mesh_Ids = std::vector<uint32_t>();

			for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
				scene_node->Mesh_Ids.push_back(node->mMeshes[i]);
			}

			DirectX::XMFLOAT4X4 matrix = DirectX::XMFLOAT4X4(
				node->mTransformation.a1, node->mTransformation.a2, node->mTransformation.a3, node->mTransformation.a4,
				node->mTransformation.b1, node->mTransformation.b2, node->mTransformation.b3, node->mTransformation.b4,
				node->mTransformation.c1, node->mTransformation.c2, node->mTransformation.c3, node->mTransformation.c4,
				node->mTransformation.d1, node->mTransformation.d2, node->mTransformation.d3, node->mTransformation.d4
			);

			scene_node->Transform = DirectX::XMLoadFloat4x4(&matrix);
			//scene_node->Transform = DirectX::XMMatrixTranspose(scene_node->Transform);

			scene_node->Children = std::vector<Scene::SceneNode*>(node->mNumChildren);
			scene_node->Parent = parent;

			if (parent) {
				scene_node->Inverse = DirectX::XMMatrixInverse(nullptr, scene_node->Transform);
			}

			for (uint32_t i = 0; i < node->mNumChildren; ++i) {
				ParseAssimpSceneTree(node->mChildren[i], scene_node->Children[i], scene_node);
			}
		}
	private:
		MeshManager& m_MeshManager;
		MaterialManager& m_MaterialManager;
		std::unordered_map<std::string, SceneRef> m_SceneCache;
	};
}
#endif