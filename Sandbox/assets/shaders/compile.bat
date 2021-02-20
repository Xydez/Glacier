@ECHO OFF
glslc -fshader-stage=vertex -o vertex.spv -Werror vertex.glsl
IF %ERRORLEVEL% NEQ 0 pause
glslc -fshader-stage=fragment -o fragment.spv -Werror fragment.glsl
IF %ERRORLEVEL% NEQ 0 pause
