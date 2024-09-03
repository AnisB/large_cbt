bacasable_exe(outer_space "projects" "outer_space.cpp" "${BACASABLE_SDK_INCLUDE};${CBF_SDK_INCLUDE};${DEMO_SDK_INCLUDE};")

# Our static libraries
target_link_libraries(outer_space "demo" "d3d12.lib;dxgi.lib;d3dcompiler.lib")

# Zlib
target_link_libraries(outer_space debug "${PROJECT_3RD_LIBRARY}/Debug/dxcompiler.lib")
target_link_libraries(outer_space optimized "${PROJECT_3RD_LIBRARY}/Release/dxcompiler.lib")

# Zlib
target_link_libraries(outer_space debug "${PROJECT_3RD_LIBRARY}/Debug/zlib.lib")
target_link_libraries(outer_space optimized "${PROJECT_3RD_LIBRARY}/Release/zlib.lib")

# Spng
target_link_libraries(outer_space debug "${PROJECT_3RD_LIBRARY}/Debug/spng.lib")
target_link_libraries(outer_space optimized "${PROJECT_3RD_LIBRARY}/Release/spng.lib")

# TinyTIFF
target_link_libraries(outer_space debug "${PROJECT_3RD_LIBRARY}/Debug/tiffd.lib")
target_link_libraries(outer_space debug "${PROJECT_3RD_LIBRARY}/Debug/tiffxxd.lib")
target_link_libraries(outer_space optimized "${PROJECT_3RD_LIBRARY}/Release/tiff.lib")
target_link_libraries(outer_space optimized "${PROJECT_3RD_LIBRARY}/Release/tiffxx.lib")

# DLLs
copy_next_to_binary(outer_space "${PROJECT_3RD_BINARY}/dxcompiler.dll")
copy_next_to_binary(outer_space "${PROJECT_3RD_BINARY}/dxil.dll")
copy_next_to_binary(outer_space "${PROJECT_3RD_BINARY}/tiff.dll")
copy_next_to_binary(outer_space "${PROJECT_3RD_BINARY}/tiffd.dll")
copy_next_to_binary(outer_space "${PROJECT_3RD_BINARY}/msvcp140.dll")
copy_next_to_binary(outer_space "${PROJECT_3RD_BINARY}/vcruntime140.dll")
copy_next_to_binary(outer_space "${PROJECT_3RD_BINARY}/vcruntime140_1.dll")

# Agility SDK
copy_dir_next_to_binary(outer_space "${PROJECT_SOURCE_DIR}/3rd/bin/D3D12" "D3D12")

# Set the argmuent
set_target_properties(outer_space PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "${PROJECT_SOURCE_DIR}")