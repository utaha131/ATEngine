#ifndef _AT_MATERIAL_SYSTEM_H_
#define _AT_MATERIAL_SYSTEM_H_
#include "../RenderSystem/GPUResources.h"
#include "../RenderSystem/GPUShader.h"
#include <unordered_map>
#include <string>
#include "../RenderSystem/GPUConstantBuffer.h"
#include "../RenderSystem/GPUResourceManager.h"
#include "../ResourceManagers/ImageManager.h"

namespace AT {
	struct Material {
		std::string BaseColorPath;
		std::string NormalPath;
		std::string RoughnessMetalnessPath;

		//Factor
		struct MaterialFactor {
			DirectX::XMFLOAT3 BaseColorFactor;
			uint32_t padding;
			DirectX::XMFLOAT3 RoughenssMetalnessFactor;
		} Factors;
		GPUConstantBuffer FactorConstantBuffer;
		GPUTexturePtr BaseColor;
		GPUTexturePtr Normal;
		GPUTexturePtr RoughnessMetalness;

		//SRV
		RHI::ShaderResourceView BaseColorSRV;
		RHI::ShaderResourceView NormalSRV;
		RHI::ShaderResourceView RoughnessMetalnessSRV;

		bool Uploaded = false;
	};
	class MaterialManager {
	public:
		BEGIN_SHADER_PARAMETER_GROUP(MaterialGroup)
			BEGIN_CONSTANTS
				DEFINE_CONSTANT(DirectX::XMFLOAT3, BaseColorFactor)
				DEFINE_CONSTANT(uint32_t, padding)
				DEFINE_CONSTANT(DirectX::XMFLOAT3, RoughenssMetalnessFactor)
			END_CONSTANTS
				SHADER_PARAMETER(AT::GPUShader::Texture2D, BaseColor)
				SHADER_PARAMETER(AT::GPUShader::Texture2D, Normals)
				SHADER_PARAMETER(AT::GPUShader::Texture2D, Surface)
		END_SHADER_PARAMETER_GROUP(MaterialGroup)

		MaterialManager(RHI::Device device, GPUResourceManager& resource_manager) :
			m_Device(device),
			m_ResourceManager(resource_manager)
		{
			RHI::TextureDescription empty_texture;
			empty_texture.Format = RHI::Format::R8G8B8A8_UNORM;
			empty_texture.TextureType = RHI::TextureType::TEXTURE_2D;
			empty_texture.Width = 10;
			empty_texture.Height = 10;
			empty_texture.DepthOrArray = 1;
			empty_texture.MipLevels = 1;
			empty_texture.UsageFlags = RHI::TextureUsageFlag::SHADER_RESOURCE;
			m_EmptyTexture = resource_manager.CreateTexture(empty_texture);

			RHI::ShaderResourceViewDescription srv_description;
			srv_description.Format = m_EmptyTexture->GetRHIHandle()->GetDescription().Format;
			srv_description.ViewDimension = RHI::ShaderResourceViewViewDimension::TEXTURE_2D;
			srv_description.Texture2D.MostDetailedMip = 0;
			srv_description.Texture2D.MipLevels = m_EmptyTexture->GetRHIHandle()->GetDescription().MipLevels;
			srv_description.Texture2D.PlaneSlice = 0;
			srv_description.Texture2D.ResourceMinLODClamp = 0;
			m_Device->CreateShaderResourceView(m_EmptyTexture->GetRHIHandle(), srv_description, m_EmptyTextureSRV);
			GPUResourceTransitionBatch transition;
			transition.AddTextureTransition(m_EmptyTexture, RHI::TextureState::PIXEL_SHADER_RESOURCE);
			m_ResourceManager.ExecuteTransitions(transition);
		}

		~MaterialManager() {
			for (auto material : m_Materials) {
				if (material->Uploaded) {
					if (material->BaseColorSRV != m_EmptyTextureSRV) {
						delete material->BaseColorSRV;
					}
					if (material->NormalSRV != m_EmptyTextureSRV) {
						delete material->NormalSRV;
					}
					if (material->RoughnessMetalnessSRV != m_EmptyTextureSRV) {
						delete material->RoughnessMetalnessSRV;
					}
					//delete material->Descriptor_Table;
					delete material;
				}
			}

			delete m_EmptyTextureSRV;
		}

		Material* CreateMaterial(const std::string& name, DirectX::XMFLOAT3& base_color_factor, DirectX::XMFLOAT3& roughness_metalness_factor, const std::string& base_color_path, const std::string& normal_path, const std::string& roughness_metalness_path) {
			if (m_MaterialCache.find(name) != m_MaterialCache.end()) {
				return m_MaterialCache[name];
			}
			Material* new_material = new Material{};
			new_material->Factors.BaseColorFactor = base_color_factor;
			new_material->Factors.RoughenssMetalnessFactor = roughness_metalness_factor;
			new_material->BaseColorPath = base_color_path;
			new_material->NormalPath = normal_path;
			new_material->RoughnessMetalnessPath = roughness_metalness_path;

			m_Materials.push_back(new_material);
			m_MaterialCache[name] = new_material;
			return new_material;
		}

		void UploadMaterial() {
			GPUResourceUploadBatch upload_batch;
			GPUGenerateMipsBatch generate_mips_batch;
			GPUResourceTransitionBatch initial_transition_batch;
			GPUResourceTransitionBatch final_transition_batch;
			for (auto pair : m_MaterialCache) {
				Material* material = pair.second;

				if (!material->Uploaded) {
					RHI::BufferDescription cb_description;
					cb_description.Size = ((sizeof(MaterialGroup::Constants) + 255) & ~255);
					cb_description.UsageFlags = RHI::BufferUsageFlag::UNIFORM_BUFFER;
					GPUBufferPtr buffer = m_ResourceManager.CreateUploadBuffer(cb_description);
					material->FactorConstantBuffer = GPUConstantBuffer(m_Device, buffer->GetRHIHandle());
					material->FactorConstantBuffer.WriteData(material->Factors);

					material->BaseColor = (material->BaseColorPath == "") ? m_EmptyTexture : CreateMaterialTexture(material->BaseColorPath, Image(material->BaseColorPath, true), upload_batch, initial_transition_batch, final_transition_batch, generate_mips_batch, RHI::Format::R8G8B8A8_TYPELESS);
					material->Normal = (material->NormalPath == "") ? m_EmptyTexture : CreateMaterialTexture(material->NormalPath, Image(material->NormalPath, false), upload_batch, initial_transition_batch,  final_transition_batch, generate_mips_batch, RHI::Format::R8G8B8A8_UNORM);
					material->RoughnessMetalness = (material->RoughnessMetalnessPath == "") ? m_EmptyTexture : CreateMaterialTexture(material->RoughnessMetalnessPath, Image(material->RoughnessMetalnessPath, false), upload_batch, initial_transition_batch, final_transition_batch, generate_mips_batch, RHI::Format::R8G8B8A8_UNORM);

					material->BaseColorSRV = (material->BaseColorPath == "") ? m_EmptyTextureSRV : CreateMaterialTextureSRV(material->BaseColor);
					material->NormalSRV = (material->NormalPath == "") ? m_EmptyTextureSRV : CreateMaterialTextureSRV(material->Normal);
					material->RoughnessMetalnessSRV = (material->RoughnessMetalnessPath == "") ? m_EmptyTextureSRV : CreateMaterialTextureSRV(material->RoughnessMetalness);

					MaterialGroup group;
					material->Uploaded = true;
				}
			}
			m_ResourceManager.ExecuteTransitions(initial_transition_batch);
			m_ResourceManager.UploadBatches(upload_batch);
			m_ResourceManager.WaitForIdle();
			/*for (auto buffer : m_staging_resources) {
				m_resource_manager.FreeBuffer(buffer);
			}
			m_staging_resources.clear();*/
			m_ResourceManager.GenerateMips(generate_mips_batch);
			m_ResourceManager.ExecuteTransitions(final_transition_batch);
		}
	protected:
		RHI::ShaderResourceView CreateMaterialTextureSRV(GPUTexturePtr texture) {
			RHI::ShaderResourceViewDescription srv_description;
			srv_description.Format = (texture->GetRHIHandle()->GetDescription().Format == RHI::Format::R8G8B8A8_TYPELESS) ? RHI::Format::R8G8B8A8_UNORM_SRGB : texture->GetRHIHandle()->GetDescription().Format;//;texture->rhi_texture->GetDescription().Format;
			srv_description.ViewDimension = RHI::ShaderResourceViewViewDimension::TEXTURE_2D;
			srv_description.Texture2D.MostDetailedMip = 0;
			srv_description.Texture2D.MipLevels = texture->GetRHIHandle()->GetDescription().MipLevels;
			srv_description.Texture2D.PlaneSlice = 0;
			srv_description.Texture2D.ResourceMinLODClamp = 0;
			RHI::ShaderResourceView srv;
			m_Device->CreateShaderResourceView(texture->GetRHIHandle(), srv_description, srv);
			return srv;
		}

		GPUTexturePtr CreateMaterialTexture(const std::string& path, AT::Image& image, AT::GPUResourceUploadBatch& upload_batch, AT::GPUResourceTransitionBatch& initial_transition_batch, AT::GPUResourceTransitionBatch& transition_batch, AT::GPUGenerateMipsBatch& generate_mips_batch, RHI::Format format) {
			if (m_TextureCache.find(path) != m_TextureCache.end()) {
				return m_TextureCache[path];
			}

			RHI::TextureDescription texture_description;
			texture_description.TextureType = RHI::TextureType::TEXTURE_2D;
			texture_description.Format = format;
			texture_description.Width = image.GetWidth();
			texture_description.Height = image.GetHeight();
			texture_description.DepthOrArray = 1;
			texture_description.MipLevels = (image.GetWidth() > 5) ? 11 : 1;
			texture_description.UsageFlags = RHI::TextureUsageFlag::SHADER_RESOURCE | RHI::TextureUsageFlag::UNORDERED_ACCESS;
			GPUTexturePtr texture = m_ResourceManager.CreateTexture(texture_description);

			RHI::BufferDescription staging_buffer_description;
			staging_buffer_description.Size = ((texture_description.Width + 255) & ~255) * texture_description.Height * 4;
			staging_buffer_description.UsageFlags = RHI::BufferUsageFlag::NONE;
			GPUBufferPtr staging_buffer = m_ResourceManager.CreateUploadBuffer(staging_buffer_description);
			staging_buffer->GetRHIHandle()->Map();
			staging_buffer->GetRHIHandle()->CopyData(0, image.GetBytes(), image.GetDataSize());
			upload_batch.AddTextureUpload(texture, staging_buffer);
			generate_mips_batch.AddTexture(texture);
			initial_transition_batch.AddTextureTransition(texture, RHI::TextureState::COMMON);
			transition_batch.AddTextureTransition(texture, RHI::TextureState::PIXEL_SHADER_RESOURCE);
			m_StagingResources.push_back(staging_buffer);
			m_TextureCache[path] = texture;
			return texture;
		}
	public:
		RHI::Device m_Device;
		GPUResourceManager& m_ResourceManager;
		std::unordered_map<std::string, Material*> m_MaterialCache;
		std::unordered_map<std::string, GPUTexturePtr> m_TextureCache;
		std::vector<Material*> m_Materials;
		GPUTexture* m_EmptyTexture;
		RHI::ShaderResourceView m_EmptyTextureSRV;
		std::vector<GPUBufferPtr> m_StagingResources;
	};
}
#endif