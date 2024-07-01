@echo off
set shadername=RayTrace
set shader=C:\Users\4cdyl\OneDrive\Desktop\ATEngine\shaders\hlsl\%shadername%.hlsl
set target=lib_6_3
set entry=main
set output=%shadername%.cs.spv
C:\Users\4cdyl\OneDrive\Desktop\DXC\bin\x64\dxc.exe -spirv -T %target% -E %entry% %shader% -Fo %output% -fspv-target-env=vulkan1.2 -fspv-extension=SPV_KHR_ray_tracing
pause