# Point to the local includes
set(D3D12_INCLUDE_DIRS "${PROJECT_3RD_INCLUDES}")

# List of required libraries
set(D3D12_LIBRARIES d3d12.lib dxgi.lib d3dcompiler.lib dxcompiler.lib)

# Handle the QUIETLY and REQUIRED arguments and set D3D12_FOUND to TRUE
include(FindPackageHandleStandardArgs)

# if all listed variables are TRUE
find_package_handle_standard_args(D3D12 DEFAULT_MSG D3D12_INCLUDE_DIRS D3D12_LIBRARIES)

# Hide from UI
mark_as_advanced(D3D12_INCLUDE_DIRS D3D12_LIBRARIES)