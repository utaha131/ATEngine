@echo off
set shadername=GenerateMipChain
set shader=C:\Users\4cdyl\OneDrive\Desktop\ATEngine\shaders\hlsl\%shadername%.hlsl
set target=cs_6_0
set entry=main
set output=%shadername%.cs.spv
C:\Users\4cdyl\OneDrive\Desktop\DXC\bin\x64\dxc.exe -spirv -T %target% -E %entry% %shader% -Fo %output%
pause