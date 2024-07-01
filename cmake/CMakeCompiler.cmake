# This module is shared; use include blocker.
if( _COMPILER_ )
	return()
endif()

# Mark it as processed
set(_COMPILER_ 1)

# Initialize MSVC version
if( "${CMAKE_GENERATOR}" MATCHES "Visual Studio" AND NOT MSVC_VERSION )
	# Reset the mscv flags
	set(MSVC10)
	set(MSVC11)
	set(MSVC12)
	set(MSVC14)
	set(MSVC 1)
	set(MSVC_IDE 1)

	if( "${CMAKE_GENERATOR}" MATCHES "Visual Studio 14 2015" )
		set(MSVC14 1)
		set(MSVC_VERSION 1900)
	else()
		message(FATAL_ERROR "Unsupported Visual Studio version!")
	endif()
endif()

message(STATUS ${MSVC_VERSION})
if( "${CMAKE_GENERATOR}" MATCHES "Visual Studio" )
	if( MSVC_VERSION EQUAL 1900 )
		set(DEVENV_NAME "vs2015")
		set(COMPILER_NAME "vc14")
	elseif( MSVC_VERSION GREATER_EQUAL 1911 )
		set(DEVENV_NAME "vs2017")
		set(COMPILER_NAME "vc15")
	else()
		message(FATAL_ERROR "Missing Visual Studio version defines!")
	endif()
endif()
