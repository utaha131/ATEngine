#ifndef _AT_RENDER_SYSTEM_MESH_MANAGER_H_
#define _AT_RENDER_SYSTEM_MESH_MANAGER_H_
#include "VertexFormats.h"
#include <vector>
#include "../RHI/RHI.h"
#include <unordered_map>
#include <string>
#include "GPUResourceManager.h"

namespace AT {

	struct Mesh {
		//CPU Data.
		std::vector<AT::VertexFormat::Vertex> Vertices;
		std::vector<AT::VertexFormat::Index> Indices;
		uint32_t MaterialID;
		uint64_t IndicesCount;

		//GPU Data.
		GPUBufferPtr VertexBuffer;
		GPUBufferPtr IndexBuffer;
		RHI::VertexBufferView VBView;
		RHI::IndexBufferView IBView;
		bool Uploaded = false;
	};

	class MeshManager {
	public:
		MeshManager(GPUResourceManager& resource_manager) :
			m_ResourceManager(resource_manager)
		{

		}

		Mesh* CreateMesh(const std::string& name, const std::vector<AT::VertexFormat::Vertex>& vertices, const std::vector<AT::VertexFormat::Index>& indices, uint32_t material_id) {
			if (m_MeshCache.find(name) != m_MeshCache.end()) {
				return m_MeshCache[name];
			}

			Mesh* new_mesh = new Mesh{};
			new_mesh->Vertices = vertices;
			new_mesh->Indices = indices;
			new_mesh->MaterialID = material_id;
			new_mesh->IndicesCount = indices.size();
			m_MeshCache[name] = new_mesh;
			return new_mesh;
		}

		void UploadMeshes() {
			GPUResourceUploadBatch upload_batch;
			GPUResourceTransitionBatch final_transition_batch;
			for (auto pair : m_MeshCache) {
				Mesh* mesh = pair.second;

				if (!mesh->Uploaded) {
					RHI::BufferDescription vertex_buffer_description;
					vertex_buffer_description.Size = mesh->Vertices.size() * sizeof(AT::VertexFormat::Vertex);
					vertex_buffer_description.UsageFlags = RHI::BufferUsageFlag::NONE;
					mesh->VertexBuffer = m_ResourceManager.CreateBuffer(vertex_buffer_description);
					GPUBufferPtr staging_vertex_buffer = m_ResourceManager.CreateUploadBuffer(vertex_buffer_description);
					staging_vertex_buffer->GetRHIHandle()->Map();
					staging_vertex_buffer->GetRHIHandle()->CopyData(0, mesh->Vertices.data(), mesh->Vertices.size() * sizeof(AT::VertexFormat::Vertex));
					upload_batch.AddBufferUpload(mesh->VertexBuffer, staging_vertex_buffer);

					RHI::BufferDescription index_buffer_description;
					index_buffer_description.Size = mesh->Indices.size() * sizeof(AT::VertexFormat::Index);
					index_buffer_description.UsageFlags = RHI::BufferUsageFlag::NONE;
					mesh->IndexBuffer = m_ResourceManager.CreateBuffer(index_buffer_description);
					GPUBufferPtr staging_index_buffer = m_ResourceManager.CreateUploadBuffer(index_buffer_description);
					staging_index_buffer->GetRHIHandle()->Map();
					staging_index_buffer->GetRHIHandle()->CopyData(0, mesh->Indices.data(), mesh->Indices.size() * sizeof(AT::VertexFormat::Index));
					upload_batch.AddBufferUpload(mesh->IndexBuffer, staging_index_buffer);

					mesh->VBView.Buffer = mesh->VertexBuffer->GetRHIHandle();
					mesh->VBView.Size = vertex_buffer_description.Size;
					mesh->VBView.Stride = sizeof(AT::VertexFormat::Vertex);

					mesh->IBView.Buffer = mesh->IndexBuffer->GetRHIHandle();
					mesh->IBView.Format = RHI::Format::R16_UINT;
					mesh->IBView.Size = index_buffer_description.Size;

					m_StagingResources.push_back(staging_vertex_buffer);
					m_StagingResources.push_back(staging_index_buffer);

					final_transition_batch.AddBufferTransition(mesh->VertexBuffer, RHI::BufferState::VERTEX_BUFFER);
					final_transition_batch.AddBufferTransition(mesh->IndexBuffer, RHI::BufferState::INDEX_BUFFER);
					mesh->Uploaded = true;
				}
			}

			m_ResourceManager.UploadBatches(upload_batch);
			m_ResourceManager.ExecuteTransitions(final_transition_batch);
			m_ResourceManager.WaitForIdle();
			for (auto buffer : m_StagingResources) {
				m_ResourceManager.FreeBuffer(buffer);
			}
			m_StagingResources.clear();
		}
	private:
		GPUResourceManager& m_ResourceManager;
		std::unordered_map<std::string, Mesh*> m_MeshCache;
		std::vector<GPUBufferPtr> m_StagingResources;
	};
}
#endif