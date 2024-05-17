# ATEngine

ATEngine is a cross-platform real time rendering engine. The goal of this project is to learn and implement a real-time rendering engine, rendering techniques, and optimizations.

# Key Features
- Cross-Platform Extenable Rendering Hardware Interface (DirectX 12 and Vulkan supported).
- Render Graph Architecture & Multihreaded Command List Recording.
- Multithreaded Job System.
- Physically Based Rendering (PBR) Material System.
- Deferred Shading, SSAO, Directional and Point Light Shadow Mapping, Reflection & Light Probes, etc.

# Design
![Architecture Design](https://raw.githubusercontent.com/utaha131/ATEngine/main/Screenshots/Architecture_Image.png)

# Implementation Status
| Feature | Description | Status|
| :---    |    :---:    |  ---: |
| Rendering Hardware Interface (RHI) | [More Details](https://github.com/utaha131/ATEngine/tree/main/RHI) | Passing |
| Render Graph Architecture | Frame task graph system for scheduling render operations. |  Working |
| PBR Materials | PBR Materials with BRDF based on UE4 | Completed |
| Forward Shading | Forward Rendering Pipeline. | Completed |
| Deferred Shading | Deferred Rendering Pipeline. | Completed |
| SSAO | Screen-Space Ambient Occlusion. | Completed |
| Cascaded Shadow Mapping | Shadow's for Directional Light. | Completed |
| Omnidirectional Shadow Mapping | Shadow's for Point Light. | Completed |
| PCF Filtering | PCF Filtering for Point and Directional Light Shadows. | Completed |
| Light Probe | Diffuse Image-Based Lighting. | Completed |
| Reflection Probe | Specular IBL using Split-Sum Approximation and Importance Sampling. | Completed |
| Tone Mapping | Reinhard & ACES Film | Completed |
| Job System | Multithreaded Job System based on Naughty Dog's presentation. | Completed |
| Parallel Command List Recording | Multithreaded Rendering. | Completed |
| SSR | Screen-Space Reflections in Compute Shader. | In Progess |
| GTAO |  | TODO |
| Atmospheric Scattering |  | TODO |
| TAA | Temporal Anti-Aliasing. | TODO |
| Scene Loading | Saving and Loading Custom Scenes | TODO |

# ScreenShots
![DirectX 12 & Vulkan Support](https://raw.githubusercontent.com/utaha131/ATEngine/main/Screenshots/DX12VK.jpg)
![Cascaded Shadow Mapping](https://raw.githubusercontent.com/utaha131/ATEngine/main/Screenshots/CSM.jpg)
![Point Light & Shadow Mapping](https://raw.githubusercontent.com/utaha131/ATEngine/main/Screenshots/OSM.jpg)
![Reflection Probe](https://raw.githubusercontent.com/utaha131/ATEngine/main/Screenshots/ReflectionProbe.jpg)
![SSAO](https://raw.githubusercontent.com/utaha131/ATEngine/main/Screenshots/SSAO.jpg)

# Dependencies
- [Vulkan](https://www.vulkan.org/)
- [DirectX 12](https://learn.microsoft.com/en-us/windows/win32/direct3d12/direct3d-12-graphics)
- [SDL2](https://www.libsdl.org/) - For Creating window and handling inputs on linux.
- [Assimp](https://github.com/assimp/assimp) - For Scene and Model Loading.
- [STBI](https://github.com/nothings/stb) - For Loading Images.
- [DirectXMath](https://github.com/microsoft/DirectXMath) - 3D Math Library.

# References
## Rendering Hardware Interface
- [Graphics API abstraction](https://wickedengine.net/2021/05/06/graphics-api-abstraction/)
- [Designing a Modern Cross-Platform Low-Level Graphics Library](https://www.gamedeveloper.com/programming/designing-a-modern-cross-platform-low-level-graphics-library)
- [Halcyon Architecture](https://media.contentapi.ea.com/content/dam/ea/seed/presentations/wihlidal-halcyonarchitecture-notes.pdf)

## Engine Architecture & Components
- [Render Graphs by Riccardo Loggini](https://logins.github.io/graphics/2021/05/31/RenderGraphs.html)
- [Parallelizing the Naughty Dog Engine Using Fibers](https://www.gdcvault.com/play/1022186/Parallelizing-the-Naughty-Dog-Engine)
- [Wicked Engine Job System](https://wickedengine.net/2018/11/simple-job-system-using-standard-c/)

## Shading and Rendering techniques
- [Unreal Engine PBR](https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf)
