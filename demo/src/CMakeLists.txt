# The defines we need for the library
set(DEMO_SDK_INCLUDES ${DEMO_SDK_ROOT}/include)
set(DEMO_SDK_SOURCE ${DEMO_SDK_ROOT}/src)

sub_directory_list(sub_projects_headers "${DEMO_SDK_INCLUDES}")
foreach(header_dir ${sub_projects_headers})
	bacasable_headers(tmp_header_list "${DEMO_SDK_INCLUDES}/${header_dir}" "${header_dir}")
	list(APPEND header_files "${tmp_header_list}")
endforeach()

sub_directory_list(sub_projects_sources "${DEMO_SDK_SOURCE}")
foreach(source_dir ${sub_projects_sources})
	bacasable_sources(tmp_source_list "${DEMO_SDK_SOURCE}/${source_dir}" "${source_dir}")
	list(APPEND source_files "${tmp_source_list}")
endforeach()

# Generate the static library
bacasable_static_lib(demo "sdk" "${header_files};${source_files};" "${DEMO_SDK_INCLUDES};${PROJECT_3RD_INCLUDES};${D3D12_INCLUDE_DIRS};")