#ifndef _RHI_CORE_H_
#define _RHI_CORE_H_
#define RHI_NULL_HANDLE nullptr
#define RHI_FLOAT32_MAX 3.402823466e+38f
#include <stdint.h>
#include <vector>
#include <string>
#include <optional>
#include <exception>

namespace RHI {
	typedef class IAdapter* Adapter;
	typedef class IBuffer* Buffer;
	typedef class ICommandAllocator* CommandAllocator;
	typedef class ICommandList* CommandList;
	typedef class IConstantBufferView* ConstantBufferView;
	typedef class IDepthStencilView* DepthStencilView;
	typedef class IDescriptorTable* DescriptorTable;
	typedef class IDevice* Device;
	typedef class IFence* Fence;
	typedef class IPipelineState* PipelineState;
	typedef class IRenderBackend* RenderBackend;
	typedef class IRenderTargetView* RenderTargetView;
	typedef class IResourceHeap* ResourceHeap;
	typedef class IRootSignature* RootSignature;
	typedef class ISampler* Sampler;
	typedef class IShader* Shader;
	typedef class IShaderResourceView* ShaderResourceView;
	typedef class ISwapChain* SwapChain;
	typedef class ITexture* Texture;
	typedef class IUnorderedAccessView* UnorderedAccessView;

	enum class CommandType {
		DIRECT,
		COPY,
		COMPUTE,
		NUM_TYPES,
	};

	enum class Format {
		UNKNOWN,
		R32G32B32A32_TYPELESS,
		R32G32B32A32_FLOAT,
		R32G32B32A32_UINT,
		R32G32B32A32_SINT,

		R32G32B32_TYPELESS,
		R32G32B32_FLOAT,
		R32G32B32_UINT,
		R32G32B32_SINT,

		R16G16B16A16_TYPELESS,
		R16G16B16A16_FLOAT,
		R16G16B16A16_UNORM,
		R16G16B16A16_UINT,
		R16G16B16A16_SNORM,
		R16G16B16A16_SINT,

		R32G32_TYPELESS,
		R32G32_FLOAT,
		R32G32_UINT,
		R32G32_SINT,

		R32G8X24_TYPELESS,
		D32_FLOAT_S8X24_UINT,
		R32_FLOAT_X8X24_TYPELESS,
		X32_TYPELESS_G8X24_UINT,

		R10G10B10A2_TYPELESS,
		R10G10B10A2_UNORM,
		R10G10B10A2_UINT,
		R11G11B10_FLOAT,

		R8G8B8A8_TYPELESS,
		R8G8B8A8_UNORM,
		R8G8B8A8_UNORM_SRGB,
		R8G8B8A8_UINT,
		R8G8B8A8_SNORM,
		R8G8B8A8_SINT,

		R16G16_TYPELESS,
		R16G16_FLOAT,
		R16G16_UNORM,
		R16G16_UINT,
		R16G16_SNORM,
		R16G16_SINT,

		R32_TYPELESS,
		D32_FLOAT,
		R32_FLOAT,
		R32_UINT,
		R32_SINT,

		R24G8_TYPELESS,
		D24_UNORM_S8_UINT,
		R24_UNORM_X8_TYPELESS,
		X24_TYPELESS_G8_UINT,

		R8G8_TYPELESS,
		R8G8_UNORM,
		R8G8_UINT,
		R8G8_SNORM,
		R8G8_SINT,

		R16_TYPELESS,
		R16_FLOAT,
		D16_UNORM,
		R16_UNORM,
		R16_UINT,
		R16_SNORM,
		R16_SINT,

		R8_TYPELESS,
		R8_UNORM,
		R8_UINT,
		R8_SNORM,
		R8_SINT,
		A8_UNORM,

		R1_UNORM,

		R9G9B9E5_SHAREDEXP,

		R8G8_B8G8_UNORM,

		G8R8_G8B8_UNORM,

		BC1_TYPELESS,
		BC1_UNORM,
		BC1_UNORM_SRGB,

		BC2_TYPELESS,
		BC2_UNORM,
		BC2_UNORM_SRGB,

		BC3_TYPELESS,
		BC3_UNORM,
		BC3_UNORM_SRGB,

		BC4_TYPELESS,
		BC4_UNORM,
		BC4_SNORM,

		BC5_TYPELESS,
		BC5_UNORM,
		BC5_SNORM,

		B5G6R5_UNORM,

		B5G5R5A1_UNORM,

		B8G8R8A8_UNORM,

		B8G8R8X8_UNORM,

		R10G10B10_XR_BIAS_A2_UNORM,

		B8G8R8A8_TYPELESS,
		B8G8R8A8_UNORM_SRGB,

		B8G8R8X8_TYPELESS,
		B8G8R8X8_UNORM_SRGB,

		BC6H_TYPELESS,
		BC6H_UF16,
		BC6H_SF16,

		BC7_TYPELESS,
		BC7_UNORM,
		BC7_UNORM_SRGB,
	};

	inline bool RHI_Format_Is_Depth(RHI::Format format) {
		switch (format) {
		case RHI::Format::D32_FLOAT_S8X24_UINT:
		case RHI::Format::R32G8X24_TYPELESS:
		case RHI::Format::D32_FLOAT:
		case RHI::Format::D24_UNORM_S8_UINT:
		case RHI::Format::D16_UNORM:
			return true;
		default:
			return false;
		}
	}

	inline bool RHI_Format_Is_Typeless(RHI::Format format) {
		switch (format) {
		case RHI::Format::R32G32B32A32_TYPELESS:
		case RHI::Format::R32G32B32_TYPELESS:
		case RHI::Format::R16G16B16A16_TYPELESS:
		case RHI::Format::R32G32_TYPELESS:
		case RHI::Format::R32G8X24_TYPELESS:
		case RHI::Format::R10G10B10A2_TYPELESS:
		case RHI::Format::R8G8B8A8_TYPELESS:
		case RHI::Format::R16G16_TYPELESS:
		case RHI::Format::R32_TYPELESS:
		case RHI::Format::R24G8_TYPELESS:
		case RHI::Format::R8G8_TYPELESS:
		case RHI::Format::R16_TYPELESS:
		case RHI::Format::R8_TYPELESS:
		case RHI::Format::BC1_TYPELESS:
		case RHI::Format::BC2_TYPELESS:
		case RHI::Format::BC3_TYPELESS:
		case RHI::Format::BC4_TYPELESS:
		case RHI::Format::BC5_TYPELESS:
		case RHI::Format::B8G8R8A8_TYPELESS:
		case RHI::Format::B8G8R8X8_TYPELESS:
		case RHI::Format::BC6H_TYPELESS:
		case RHI::Format::BC7_TYPELESS:
			return true;
		default:
			return false;
		}
	}

	typedef struct Rect {
		int32_t Top;
		int32_t Left;
		int32_t Bottom;
		int32_t Right;
	} Rect;

	typedef struct Viewport {
		float TopLeftX;
		float TopLeftY;
		float Width;
		float Height;
		float MinDepth;
		float MaxDepth;
	} Viewport;


	typedef struct InputElement {
		std::string Name;
		RHI::Format Format;
		uint32_t Offset;
	} InputElement;

	typedef struct InputLayout {
		std::vector<InputElement> InputElements;
		uint32_t InputStride;
	} InputLayout;

	enum class DepthWriteMask {
		ZERO,
		ALL,
	};

	enum class ComparisonFunction {
		NEVER,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_EQUAL,
		ALWAYS,
	};

	enum StencilOperation {
		KEEP,
		ZERO,
		REPLACE,
		INCREASE_SATURATE,
		DECREASE_SATURATE,
		INVERT,
		INCREASE,
		DECREASE,
	};

	typedef struct DepthStencilOperationDescription {
		RHI::StencilOperation StencilFailOperation;
		RHI::StencilOperation StencilDepthFailOperation;
		RHI::StencilOperation StencilPassOperation;
		RHI::ComparisonFunction ComparisonFunction;
	} DepthStencilOperationDescription;

	typedef struct DepthStencilDescription {
		bool DepthEnabled;
		RHI::DepthWriteMask DepthWriteMask;
		RHI::ComparisonFunction ComparisonFunction;
		bool StencilEnabled;
		uint8_t StencilReadMask;
		uint8_t StencilWriteMask;
		RHI::DepthStencilOperationDescription DepthStencilOperationDescription;
	} DepthStencilDescription;

	enum class FillMode {
		WIREFRAME,
		SOLID,
	};

	enum class CullMode {
		NONE,
		FRONT,
		BACK,
	};

	typedef struct RasterizerDescription {
		RHI::FillMode FillMode;
		RHI::CullMode CullMode;
		bool FrontCounterClockwise;
	} RasterizeDescription;

	enum class Blend : uint8_t {
		ZERO,
		ONE,
		SOURCE_COLOR,
		INVERSE_SOURCE_COLOR,
		SOURCE_ALPHA,
		INVERSE_SOURCE_ALPHA,
		DESTINATION_ALPHA,
		INVERSE_DESTINATION_ALPHA,
		DESTINATION_COLOR,
		INVERSE_DESTINATION_COLOR,
		SOURCE_ALPHA_SATURATE,
		BLEND_FACTOR,
		INVERSE_BLEND_FACTOR,
		SOURCE1_COLOR,
		INVERSE_SOURCE1_COLOR,
		SOURCE1_ALPHA,
		INVERSE_SOURCE1_ALPHA,
	};

	enum class BlendOperation : uint8_t {
		ADD,
		SUBTRACT,
		REVERSE_SUBTRACT,
		MIN,
		MAX,
	};

	enum class LogicOperation : uint8_t {
		CLEAR,
		SET,
		COPY,
		COPY_INVERTED,
		NOOP,
		INVERT,
		AND,
		NAND,
		OR,
		NOR,
		XOR,
		EQUIVALENT,
		AND_REVERSE,
		AND_INVERTED,
		OR_REVERSE,
		OR_INVERTED
	};

	enum class ColorWriteEnable : uint8_t {
		RED = 1 << 0,
		GREEN = 1 << 1,
		BLUE = 1 << 2,
		ALPHA = 1 << 3,
		ALL = RED | GREEN | BLUE | ALPHA,
	};

	typedef struct RenderTargetBlendDescription {
		bool BlendEnable;
		bool BlendLogicEnable;
		RHI::Blend SourceBlend;
		RHI::Blend DestinationBlend;
		RHI::BlendOperation BlendOperation;
		RHI::Blend SourceBlendAlpha;
		RHI::Blend DestinationBlendAlpha;
		RHI::BlendOperation BlendOperationAlpha;
		RHI::LogicOperation LogicOperation;
		RHI::ColorWriteEnable RenderTargetWriteMask;
	} RenderTargetBlendDescription;

	typedef struct BlendDescription {
		bool AlphaToCoverageEnable;
		bool IndependentBlendEnable;
		RHI::RenderTargetBlendDescription RenderTargetBlendDescription[8];
	} BlendDescription;

	//enum PRIMITIVE_TOPOLOGY_TYPE {
	//	PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,
	//	PRIMITIVE_TOPOLOGY_TYPE_POINT,
	//	PRIMITIVE_TOPOLOGY_TYPE_LINE,
	//	PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
	//	PRIMITIVE_TOPOLOGY_TYPE_PATCH,
	//};

	enum class PrimitiveTopology : uint8_t {
		UNDEFINED,
		POINT_LIST,
		LINE_LIST,
		LINE_STRIP,
		TRIANGLE_LIST,
		TRIANGLE_STRIP,
	};

	typedef struct SampleDescription {
		uint32_t Count;
		uint32_t Quality;
	} SampleDescription;

	enum class Filter : uint8_t {
		MIN_MAG_MIP_POINT,
		MIN_MAG_POINT_MIP_LINEAR,
		MIN_POINT_MAG_LINEAR_MIP_POINT,
		MIN_POINT_MAG_MIP_LINEAR,
		MIN_LINEAR_MAG_MIP_POINT,
		MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		MIN_MAG_LINEAR_MIP_POINT,
		MIN_MAG_MIP_LINEAR,
		ANISOTROPIC,
		COMPARISON_MIN_MAG_MIP_POINT,
		COMPARISON_MIN_MAG_POINT_MIP_LINEAR,
		COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,
		COMPARISON_MIN_POINT_MAG_MIP_LINEAR,
		COMPARISON_MIN_LINEAR_MAG_MIP_POINT,
		COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
		COMPARISON_MIN_MAG_MIP_LINEAR,
		COMPARISON_ANISOTROPIC,
		MINIMUM_MIN_MAG_MIP_POINT,
		MINIMUM_MIN_MAG_POINT_MIP_LINEAR,
		MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
		MINIMUM_MIN_POINT_MAG_MIP_LINEAR,
		MINIMUM_MIN_LINEAR_MAG_MIP_POINT,
		MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		MINIMUM_MIN_MAG_LINEAR_MIP_POINT,
		MINIMUM_MIN_MAG_MIP_LINEAR,
		MINIMUM_ANISOTROPIC,
		MAXIMUM_MIN_MAG_MIP_POINT,
		MAXIMUM_MIN_MAG_POINT_MIP_LINEAR,
		MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT,
		MAXIMUM_MIN_POINT_MAG_MIP_LINEAR,
		MAXIMUM_MIN_LINEAR_MAG_MIP_POINT,
		MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR,
		MAXIMUM_MIN_MAG_LINEAR_MIP_POINT,
		MAXIMUM_MIN_MAG_MIP_LINEAR,
		MAXIMUM_ANISOTROPIC,
	};

	enum class TextureAddressMode {
		WRAP,
		MIRROR,
		CLAMP,
		BORDER,
		MIRROR_ONCE,
	};

	typedef struct TextureClearValue {
		RHI::Format Format;
		union {
			float Color[4];
			struct DepthStencilClearValue {
				float Depth;
				uint8_t Stencil;
			} DepthAndStencil;
		};
	} TextureClearValue;

	enum Result {
		SUCCESS,
		E_CREATE_DEVICE,
		E_CREATE_SWAP_CHAIN,
		E_CREATE_RESOURCE_HEAP,
		E_CREATE_BUFFER,
		E_CREATE_TEXTURE,
		E_CREATE_ROOT_SIGNATURE,
		E_CREATE_SHADER,
		E_CREATE_RENDER_PASS,
		E_CREATE_FRAME_BUFFER,
		E_CREATE_PIPELINE_STATE,
		E_CREATE_VIEW_CONSTANT_BUFFER,
		E_CREATE_VIEW_DEPTH_STENCIL,
		E_CREATE_VIEW_RENDER_TARGET,
		E_CREATE_VIEW_SAMPLER,
		E_CREATE_VIEW_SHADER_RESOURCE,
		E_CREATE_VIEW_UNORDERED_ACCESS,
		E_CREATE_COMMAND_ALLOCATOR,
		E_CREATE_COMMAND_LIST
	};

	//typedef class IRenderPass* RenderPass;
	typedef class IFrameBuffer* FrameBuffer;

	enum class BufferUsageFlag : uint8_t {
		NONE = 0,
		UNIFORM_BUFFER = 1 << 0,
		UNORDERED_ACCESS = 1 << 1,
	};

	#undef GENERIC_READ
	enum class BufferState : uint8_t {
		COMMON,
		UNORDERED_ACCESS,
		COPY_DEST,
		COPY_SOURCE,
		GENERIC_READ,

		VERTEX_BUFFER,
		CONSTANT_BUFFER,
		INDEX_BUFFER,

		NON_PIXEL_SHADER_RESOURCE,
		PIXEL_SHADER_RESOURCE,
		RAYTRACING_ACCELERATION_STRUCTURE,
	};

	enum class TextureType : uint8_t {
		TEXTURE_1D,
		TEXTURE_2D,
		TEXTURE_3D
	};

	enum class TextureState : uint8_t {
		CREATED,
		COMMON,
		UNORDERED_ACCESS,
		COPY_DEST,
		COPY_SOURCE,
		GENERIC_READ,

		RENDER_TARGET,
		DEPTH_WRITE,
		DEPTH_READ,
		PRESENT,

		NON_PIXEL_SHADER_RESOURCE,
		PIXEL_SHADER_RESOURCE,
	};

	enum class TextureUsageFlag : uint8_t {
		NONE = 0,
		DEPTH_STENCIL = 1 << 0,
		RENDER_TARGET = 1 << 1,
		SHADER_RESOURCE = 1 << 2,
		UNORDERED_ACCESS = 1 << 3,
	};

#define RHI_SUBRESOURCE_INDEX_ALL 0xffffffff

	enum class ResourceBarrierType {
		TRANSITION_BARRIER_BUFFER,
		TRANSITION_BARRIER_TEXTURE,
		ALIASING_BARRIER_BUFFER,
		ALIASING_BARRIER_TEXTURE,
		UNORDERED_ACCESS_BARRIER_BUFFER,
		UNORDERED_ACCESS_BARRIER_TEXTURE,
	};

	struct Subresource {
		uint32_t MipSlice = RHI_SUBRESOURCE_INDEX_ALL;
		uint32_t MipLevels = RHI_SUBRESOURCE_INDEX_ALL;
		uint32_t ArraySlice = RHI_SUBRESOURCE_INDEX_ALL;
		bool operator==(const Subresource& other) const {
			return (MipSlice == other.MipSlice) && (MipLevels == other.MipLevels) && (ArraySlice == other.ArraySlice);
		}
	};

	const Subresource SUBRESOURCE_ALL = { RHI_SUBRESOURCE_INDEX_ALL, RHI_SUBRESOURCE_INDEX_ALL, RHI_SUBRESOURCE_INDEX_ALL };

	typedef struct ResourceBarrier {

	private:
		struct UnorderedAccessBuffer {
			RHI::Buffer Buffer;
		};

		struct UnorderedAccessTexture {
			RHI::Texture Texture;
		};

		struct AliasingBuffer {
			RHI::Buffer BeforeBuffer;
			RHI::Buffer AfterBuffer;
		};

		struct AliasingTexture {
			RHI::Texture BeforeTexture;
			RHI::Texture AfterTexture;
		};

		struct TransitionBuffer {
			RHI::Buffer Buffer;
			RHI::BufferState InitialState;
			RHI::BufferState FinalState;
		};

		struct TransitionTexture {
			RHI::Texture Texture;
			// uint32_t SubresourceIndex;
			Subresource Subresource;
			RHI::TextureState InitialState;
			RHI::TextureState FinalState;
		};
	public:

		RHI::ResourceBarrierType ResourceBarrierType;

		union {
			UnorderedAccessBuffer UnorderedAccessBarrierBuffer;
			UnorderedAccessTexture UnorderedAccessBarrierTexture;
			AliasingBuffer AliasingBarrierBuffer;
			AliasingTexture AliasingBarrierTexture;
			TransitionBuffer TransitionBarrierBuffer;
			TransitionTexture TransitionBarrierTexture = {};
		};
	} ResourceBarrier;

	typedef struct VertexBufferView {
		RHI::Buffer Buffer;
		uint32_t Size;
		uint32_t Stride;
	} VertexBufferView;

	typedef struct IndexBufferView {
		RHI::Buffer Buffer;
		uint32_t Size;
		RHI::Format Format;
	} IndexBufferView;

	typedef struct AllocationInfo {
		uint64_t Size;
		uint64_t Alignment;
	} AllocationInfo;

	typedef struct RenderPassAttachment {
		enum class LoadOperation {
			LOAD,
			CLEAR,
			DONT_CARE
		};

		enum class StoreOperation {
			STORE,
			DONT_CARE
		};

		RHI::Format Format = RHI::Format::UNKNOWN;
		LoadOperation LoadOp;
		StoreOperation StoreOp;
		LoadOperation StencilLoadOp;
		StoreOperation StencilStoreOp;
		RHI::TextureState InitialState;
		RHI::TextureState  FinalState;
	} RenderPassAttachment;


	typedef struct RenderPassDescription {
		std::vector<RenderPassAttachment> Attachments;
		std::optional<RenderPassAttachment> DepthAttachment;
	} RenderPassDescription;

	/*class IRenderPass {
	public:
		virtual ~IRenderPass() = default;
		inline const RenderPassDescription& GetDescription() const { return m_Description; };
	protected:
		RenderPassDescription m_Description;
		IRenderPass(const RenderPassDescription& description) :
			m_Description(description) {}
	};*/
	class IRenderPass {
	public:
		IRenderPass() {}
		IRenderPass(const RenderPassDescription& description) :
			m_Description(description) {}
		inline const RenderPassDescription& GetDescription() const { return m_Description; };
	protected:
		RenderPassDescription m_Description;
	};

	template<typename E> bool has_flag(E flag1, E flag2) {
		return ((uint8_t)flag1 & (uint8_t)flag2) == (uint8_t)flag2;
	}

	template<typename E> E operator&(E flag1, E flag2) {
		return (E)((uint8_t)flag1 & (uint8_t)flag2);
	}

	template<typename E> E operator|(E flag1, E flag2) {
		E value = (E)((uint8_t)flag1 | (uint8_t)flag2);
		return (E)((uint8_t)flag1 | (uint8_t)flag2);
	}

	typedef IRenderPass* RenderPass;


	inline void Throw(Result result) {
		if (result != Result::SUCCESS) {
			throw std::exception{};
		}
	}



























	//Ray Tracing Experimental
	class IRayTracingPipeline {
	public:
		virtual void* GetShaderIdentifier(const std::string& name) const = 0;
	};
	/*
	D3D12_RAYTRACING_GEOMETRY_DESC geometry_description;
	geometry_description.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometry_description.Triangles.VertexBuffer.StartAddress = static_cast<RHI::DX12::DX12Buffer*>(vertex_buffer->GetRHIHandle())->GetNative()->GetGPUVirtualAddress();
	geometry_description.Triangles.VertexBuffer.StrideInBytes = sizeof(Vertex);
	geometry_description.Triangles.VertexCount = 3;
	geometry_description.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometry_description.Triangles.IndexBuffer = static_cast<RHI::DX12::DX12Buffer*>(index_buffer->GetRHIHandle())->GetNative()->GetGPUVirtualAddress();
	geometry_description.Triangles.IndexCount = 3;
	geometry_description.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
	geometry_description.Triangles.Transform3x4 = 0;
	geometry_description.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	*/
	
	enum class RayTracingGeometryType {
		TRIANGLES,
		AABBS
	};

	enum class RayTracingGeometryFlag {
		NONE ,
#undef OPAQUE
		OPAQUE,
		NO_DUPLICATE_ANYHIT_INVOCATION 
	};

	struct RayTracingGeometryDescription {
		struct VertexBufferDescription {
			RHI::Buffer Buffer;
			RHI::Format Format;
			uint32_t Stride;
			uint64_t VertexCount;
		} VertexBuffer;

		struct IndexBufferDescription {
			RHI::Buffer Buffer;
			RHI::Format Format;
			uint32_t IndexCount;
		} IndexBuffer;
	};

	struct RayTracingAccelerationStructureMemoryInfo {
		uint64_t ScratchDataSize;
		uint64_t DestinationDataSize;
	};

	//Bottom Level Acceleration Structure.
	struct RayTracingBottomLevelAccelerationStructureDescription {
		std::vector<RayTracingGeometryDescription> Geometries;
	};

	class IRayTracingBottomLevelAccelerationStructure {
	public:
		IRayTracingBottomLevelAccelerationStructure(RHI::Buffer buffer) :
			m_Buffer(buffer)
		{

		}

		virtual ~IRayTracingBottomLevelAccelerationStructure() = default;
		RHI::Buffer GetBuffer() const {
			return m_Buffer;
		}
	protected:
		RHI::Buffer m_Buffer;
	};

	struct BuildRayTracingBottomLevelAccelerationStructure {
		RayTracingBottomLevelAccelerationStructureDescription* description;
		IRayTracingBottomLevelAccelerationStructure* ScratchBottomLevelAccelerationStructure;
		IRayTracingBottomLevelAccelerationStructure* DestinationBottomLevelAccelerationStructure;
	};

	//Top Level Acceleration Structure.
	/*
	D3D12_RAYTRACING_INSTANCE_DESC instance_desc = {};
	instance_desc.InstanceID = 0;
	instance_desc.InstanceContributionToHitGroupIndex = 0;
	instance_desc.InstanceMask = 0xFF;
	instance_desc.Transform[0][0] = instance_desc.Transform[1][1] = instance_desc.Transform[2][2] = 1;
	instance_desc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
	instance_desc.AccelerationStructure = static_cast<RHI::DX12::DX12Buffer*>(blas)->GetNative()->GetGPUVirtualAddress();
	RHI::BufferDescription buffer_description;
	buffer_description.Size = sizeof(instance_desc);
	buffer_description.UsageFlags = RHI::BufferUsageFlag::NONE;
	AT::GPUBufferPtr instance_desc_buffer = resource_manager.CreateUploadBuffer(buffer_description);
	instance_desc_buffer->GetRHIHandle()->Map();
	instance_desc_buffer->GetRHIHandle()->CopyData(0, &instance_desc, sizeof(instance_desc));
	*/

	struct RayTracingInstance {
		float Transform[3][4];
		uint32_t InstanceID;
		uint32_t InstanceMask;
		uint32_t InstanceContributionToHitGroupIndex;
		uint32_t Flags;
		IRayTracingBottomLevelAccelerationStructure* BottomLevelAccelerationStructure;
	};

	class IRayTracingInstanceBuffer {
	public:
		IRayTracingInstanceBuffer(RHI::Buffer buffer, uint64_t capacity) :
			m_Buffer(buffer),
			m_Capacity(capacity)
		{
			
		}

		RHI::Buffer GetBuffer() const {
			return m_Buffer;
		}

		virtual void WriteInstance(uint64_t offset, RayTracingInstance& instance) = 0;
		virtual void WriteInstances(uint64_t offset, std::vector<RayTracingInstance>& instances) = 0;
	protected:
		RHI::Buffer m_Buffer;
		uint64_t m_Capacity;
	};

	struct RayTracingTopLevelAccelerationStructureDescription {
		IRayTracingInstanceBuffer* InstancesBuffer;
		uint64_t InstanceCount;
	};

	class IRayTracingTopLevelAccelerationStructure {
	public:
		IRayTracingTopLevelAccelerationStructure(RHI::Buffer buffer) :
			m_Buffer(buffer)
		{

		}

		virtual ~IRayTracingTopLevelAccelerationStructure() = default;
		RHI::Buffer GetBuffer() const {
			return m_Buffer;
		}
	protected:
		RHI::Buffer m_Buffer;
	};

	struct BuildRayTracingTopLevelAccelerationStructure {
		RayTracingTopLevelAccelerationStructureDescription* description;
		IRayTracingTopLevelAccelerationStructure* ScratchTopLevelAccelerationStructure;
		IRayTracingTopLevelAccelerationStructure* DestinationTopLevelAccelerationStructure;
	};


	typedef struct RayTracingShaderConfiguration {
		uint32_t MaxPayloadSizeInBytes;
		uint32_t MaxAttributeSizeInBytes;
	};

	class RayTracingShaderLibrary {
	public:
		void SetLibrary(RHI::Shader shader) {
			m_Shader = shader;
		}

		void DefineExport(const std::string& export_name) {
			m_ExportNames.emplace_back(export_name);
		}
		Shader m_Shader;
		std::vector<std::string> m_ExportNames;
	};

	enum class RayTracingHitGroupType {
		TRIANGLES,
		PROCEDURAL_PRIMITIVE
	};

	typedef struct RayTracingHitGroup {
		std::string Name;
		RayTracingHitGroupType Type;
		std::string AnyHitShaderImport;
		std::string ClosestHitShaderImport;
		std::string IntersectionShaderImport;
	} HitGroup;

	enum class RayTracingExportAssociationType {
		SHADER_PAYLOAD,
		LOCAL_ROOT_SIGNATURE
	};
	typedef struct RayTracingExportAssociation {
		std::vector<std::string> ExportNames;
		RayTracingExportAssociationType Type;
		uint32_t AssociationObjectIndex;
	};

	typedef struct RayTracingPipelineStateDescription {
		std::vector<RayTracingShaderLibrary> ShaderLibraries;
		std::vector<RayTracingHitGroup> HitGroups;
		RayTracingShaderConfiguration ShaderConfiguration;
		RootSignature GlobalRootSignature;
		std::vector<RootSignature> LocalRootSignatures;
		std::vector<RayTracingExportAssociation> ExportAssociations;
		uint32_t MaxTraceRecursionDepth;
	} RayTracingPipelineStateDescription;


	struct GPUBufferRange {
		RHI::Buffer Buffer;
		uint64_t Offset;
		uint64_t Size;
	};

	struct GPUBufferRangeAndStride {
		RHI::Buffer Buffer;
		uint64_t Offset;
		uint64_t Size;
		uint64_t Stride;
	};

	struct RayTracingDispatchRaysDescription {
		uint32_t Width;
		uint32_t Height;
		uint32_t Depth;
		GPUBufferRange RayGenerationShaderRecord;
		GPUBufferRangeAndStride MissShaderTable;
		GPUBufferRangeAndStride HitGroupTable;
	};
}
#endif