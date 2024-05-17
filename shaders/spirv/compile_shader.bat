@echo off
set shadername=Depth
set shader=C:\Users\4cdyl\OneDrive\Desktop\ATEngine\shaders\hlsl\%shadername%.hlsl
set target=ps_6_0
set entry=PS
set output=%shadername%.ps.spv
C:\Users\4cdyl\OneDrive\Desktop\DXC\bin\x64\dxc.exe -spirv -T %target% -E %entry% %shader% -Fo %output%

set target=vs_6_0
set entry=VS
set output=%shadername%.vs.spv
C:\Users\4cdyl\OneDrive\Desktop\DXC\bin\x64\dxc.exe -spirv -T %target% -E %entry% %shader% -Fo %output%
pause
