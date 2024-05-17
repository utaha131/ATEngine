# RHI
RHI (Rendering Hardware Interface) is library that implements a extensible abstraction layer over modern graphics APIs like DirectX 12 & Vulkan.

The library's interfaces is mostly based on DirectX 12's API and can be seen as a wrapper over DirectX 12 with some Vulkan restrictions to provide a common abstraction layer.

# Tutorial
Backend Initialization & Device Creation
```
RHI::RenderBackend render_backend;
bool DEBUG = true;
RHI::CreateRenderBackend(api, DEBUG, render_backend);
std::vector<RHI::Adapter> v_adapters = std::vector<RHI::Adapter>();
RHI::Adapter selected_adapter = RHI_NULL_HANDLE;
render_backend->GetAdapters(v_adapters);
for (uint32_t i = 0; i < v_adapters.size(); ++i) {
    if (v_adapters[i]->GetVendor() == RHI::Vendor::NVIDIA) {
        selected_adapter = v_adapters[i];
    }
}
RHI::Device device;
render_backend->CreateDevice(selected_adapter, device);
```
Buffer Creation
```
RHI::BufferDescription description;
description.Size = 256;
description.UsageFlags = RHI::BufferUsageFlag::NONE;
RHI:Buffer buffer;
device->CreateCommittedBuffer(RHI::ResourceHeapType::DEFAULT, RHI::BufferState::COMMON, description, buffer);
```
Texture Creation
```
RHI::TextureDescription description;
description.Format = RHI::Format::R8G8B8A8_UNORM;
description.TextureType = RHI::TextureType::TEXTURE_2D;
description.Width = 100;
description.Height = 100;
description.DepthOrArray = 1;
description.MipLevels = 1;
description.UsageFlags = RHI::TextureUsageFlag::RENDER_TARGET;
RHI::Texture texture;
device->CreateCommittedTexture(RHI::ResourceHeapType::DEFAULT, clear_value, description, texture);
```

# Implementation Status & TODOs
| Feature | Description | Status|
| :---    |    :---:    |  ---: |
| Render Backend | Different Platform Handling. | Passing. Need to add Querying for platform information.|
| Resource Heap | | Passing.Need to add Readback heap support. |
| Buffer Interface | | Done |
| Texture Interface | | Done |
| SwapChain Interface | | Done |
| Root Signature | | Passing. Need to add Constants supports |
| Resource Views | CBV, SRV, UAV, RTV, DSV, and Samplers. | Passing. Need to add Samplers |
| Graphics Pipeline State | | Done|
| Compute Pipeline State | | Done |
| Shaders Interface | | Done |
| Ray Tracing Pipeline | | TODO |