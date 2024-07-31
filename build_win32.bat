@echo off
if not exist build_windows (mkdir build_windows)
pushd build_windows
set src_path=..\src\
set vulkan_sdk_include_path=%VULKAN_SDK%\Include

set win32_libs= user32.lib
set win32_libs=%win32_libs% gdi32.lib

set link=       -opt:ref
set link=%link% -incremental:no

cl /nologo /FAs /I %vulkan_sdk_include_path% -DOXIDEBUG -Z7 %src_path%win32_oxigine.c %win32_libs% /link %link% -subsystem:windows,5.2

popd