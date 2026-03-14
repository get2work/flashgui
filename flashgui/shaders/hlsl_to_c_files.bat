@echo off
setlocal

REM ==== Compile HLSL to CSO ====

dxc.exe ^
  -T ps_6_0 ^
  -E main ^
  -Fo quad_ps.cso ^
  pixel.hlsl

IF ERRORLEVEL 1 (
    echo Pixel shader compile failed.
    pause
    exit /b 1
)

dxc.exe ^
  -T vs_6_0 ^
  -E main ^
  -Fo quad_vs.cso ^
  vertex.hlsl

IF ERRORLEVEL 1 (
    echo Vertex shader compile failed.
    pause
    exit /b 1
)

REM ==== Convert CSO -> extensionless files ====

bin2c.exe quad_ps.cso quad_ps quad_ps
bin2c.exe quad_vs.cso quad_vs quad_vs

echo Done.
pause
