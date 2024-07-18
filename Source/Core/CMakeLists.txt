#---------------------------------------------------------------------------------------------------
#General project settings:
add_definitions(-D_LIB -DEE_LIB -DDEUSEX=1) 
include_directories(AFTER "${CMAKE_SOURCE_DIR}/Source/Core/")

#---------------------------------------------------------------------------------------------------
# Helpers

macro(deusex_replace_cmake_cxx_flags from to)
	set(CompilerFlags
			CMAKE_CXX_FLAGS
			CMAKE_CXX_FLAGS_DEBUG
			CMAKE_CXX_FLAGS_RELEASE
			CMAKE_C_FLAGS
			CMAKE_C_FLAGS_DEBUG
			CMAKE_C_FLAGS_RELEASE
			)
	foreach(CompilerFlag ${CompilerFlags})
		string(REPLACE "${from}" "${to}" ${CompilerFlag} "${${CompilerFlag}}")
	endforeach()
endmacro()

#---------------------------------------------------------------------------------------------------
# Source files, etc

set(SRC_BASE
	"DeusExEchelonCore.cpp"
	"coreutils.cpp"
	"commandmanager.cpp"
	"demomanager.cpp"
	"hooks/hooks.cpp"
	"hooks/uengine.cpp"
)

set(HDR_BASE
	"DeusExEchelonCore_PCH.h"
	"DeusExEchelonCore.h"
	"coreutils.h"
	"commandmanager.h"
	"demomanager.h"
	"hooks/hooks.h"
)

#---------------------------------------------------------------------------------------------------
# Configure project output
add_library (DeusExEchelonCore STATIC ${SRC_BASE} ${HDR_BASE})


if(MSVC)
	add_definitions(-D_UNICODE -DUNICODE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t- /Zp4 /Zc:inline- ") #/Zp4 because Deus Ex requires it, /Zc:inline- /GR for debugging purposes.
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI /GR")
	deusex_replace_cmake_cxx_flags(/Zi /ZI)
	SET_TARGET_PROPERTIES(DeusExEchelonCore PROPERTIES VS_DEBUGGER_COMMAND "${EE_DEUSEXFOLDER}/deusex.exe")
	SET_TARGET_PROPERTIES(DeusExEchelonCore PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${EE_DEUSEXFOLDER}")
	SET_TARGET_PROPERTIES(DeusExEchelonCore PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /DEBUG:FULL /OPT:NOICF /OPT:NOREF /DYNAMICBASE:NO /LIBPATH:\"${CMAKE_LIBRARY_PATH}\"")
	set_target_properties(DeusExEchelonCore PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}>)
	target_compile_definitions(DeusExEchelonCore
		PRIVATE $<$<CONFIG:Release>:RELEASE_CONFIG>
	)
	add_definitions(/ZI)
	SET_TARGET_PROPERTIES(DeusExEchelonCore PROPERTIES LINK_FLAGS "/SAFESEH:NO")
endif(MSVC)

target_precompile_headers(DeusExEchelonCore
	PRIVATE
	"DeusExEchelonCore_PCH.h"
)

#---------------------------------------------------------------------------------------------------
# Configure IDE
source_group(TREE "${CMAKE_SOURCE_DIR}/Source/Core" PREFIX "Source Files" FILES ${SRC_BASE})
source_group(TREE "${CMAKE_SOURCE_DIR}/Source/Core" PREFIX "Header Files" FILES ${HDR_BASE})