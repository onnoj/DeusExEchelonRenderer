#---------------------------------------------------------------------------------------------------
#General project settings:
add_definitions(-D_DLL -DEE_LIBDLL -DDEUSEX=1) 
include_directories(AFTER "${CMAKE_SOURCE_DIR}/Source/WindorDrv/")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/USER/)
set(DEUSEX_INT_FILE "DeusExEchelonRendererWindowDrv.int")
#add_compile_definitions("CONVERT_TO_LEFTHANDED_COORDINATES=1")

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

macro(deusex_modify_resourcerc)
	if(DEFINED ENV{CICD_REF})
        set(VERSION_REGEX "refs/tags/v([0-9]+)\.([0-9]+)\.([0-9]+)(.*)")
        string(REGEX MATCHALL ${VERSION_REGEX} VERSION_MATCHES $ENV{CICD_REF})
        if (VERSION_MATCHES)
			message(STATUS "Versionmatches = ${VERSION_MATCHES}")
            set(VERSION_MAJOR ${CMAKE_MATCH_1})
            set(VERSION_MINOR ${CMAKE_MATCH_2})
            set(VERSION_PATCH ${CMAKE_MATCH_3})
			
			file(STRINGS "DeusExEchelonRendererWindowDrv.rc" RES_FILE_CONTENTS NEWLINE_CONSUME)
			string(REPLACE "1,2,3,4" "${VERSION_MAJOR},${VERSION_MINOR},${VERSION_PATCH},0" RES_FILE_CONTENTS "${RES_FILE_CONTENTS}")
			string(REPLACE "1.2.3.4" "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" RES_FILE_CONTENTS "${RES_FILE_CONTENTS}")
			file(WRITE "DeusExEchelonRendererWindowDrv.rc" "${RES_FILE_CONTENTS}\n")
        else()
            message(STATUS "Environment variable CICD_REF ($ENV{CICD_REF}) does not match the expected pattern.")
        endif()
    else()
        message(STATUS "Environment variable CICD_REF ($ENV{CICD_REF}) is not defined, version file not updated.")
    endif()
endmacro()

deusex_modify_resourcerc()

#---------------------------------------------------------------------------------------------------
# Source files, etc

set(SRC_BASE
	"uefacade.cpp"
	"WindowsViewport.cpp"
	"WindowsClient.cpp"
)

set(HDR_BASE
	"DeusExEchelonRendererWindowDrv_PCH.h"
	"uefacade.h"
	"WindowsViewport.h"
	"WindowsClient.h"
	"${DEUSEX_INT_FILE}"
)

#---------------------------------------------------------------------------------------------------
# Configure project output
add_library (DeusExEchelonRendererWindowDrv SHARED ${SRC_BASE} ${HDR_BASE})
target_sources(DeusExEchelonRendererWindowDrv PRIVATE DeusExEchelonRendererWindowDrv.rc)

set_source_files_properties(${HDR_BASE} PROPERTIES HEADER_FILE_ONLY 1)
find_library(LIB_DEUSEX "deusex.lib")
find_library(LIB_CORE "core.lib")
find_library(LIB_ENGINE "engine.lib")
find_library(LIB_RENDER "render.lib")
find_library(LIB_EXTENSION "extension.lib")
find_library(LIB_D3D9 "d3d9.lib")
find_library(LIB_D3DX9 "d3dx9.lib")
find_library(LIB_SDL2 "sdl2.lib")

set(DEUSEX_LIBS 
	"${LIB_DEUSEX}" "${LIB_CORE}" "${LIB_ENGINE}" "${LIB_RENDER}" "${LIB_EXTENSION}"
	"winmm.lib" "comctl32.lib"
	"${LIB_D3D9}" "${LIB_D3DX9}" "${LIB_SDL2}"
	)
target_link_libraries (DeusExEchelonRendererWindowDrv DeusExEchelonCore ${DEUSEX_LIBS})
target_link_libraries(DeusExEchelonRendererWindowDrv PolyHook_2)

if(MSVC)
	add_definitions(-D_UNICODE -DUNICODE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t- /Zp4 /Zc:inline- ") #/Zp4 because Deus Ex requires it, /Zc:inline- /GR for debugging purposes.
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI /GR")
	deusex_replace_cmake_cxx_flags(/Zi /ZI)
	SET_TARGET_PROPERTIES(DeusExEchelonRendererWindowDrv PROPERTIES VS_DEBUGGER_COMMAND "${EE_DEUSEXFOLDER}/deusexwindowdrv.exe")
	SET_TARGET_PROPERTIES(DeusExEchelonRendererWindowDrv PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${EE_DEUSEXFOLDER}")
	SET_TARGET_PROPERTIES(DeusExEchelonRendererWindowDrv PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /DEBUG:FULL /OPT:NOICF /OPT:NOREF /DYNAMICBASE:NO /LIBPATH:\"${CMAKE_LIBRARY_PATH}\"")
	set_target_properties(DeusExEchelonRendererWindowDrv PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}>)
	target_compile_definitions(DeusExEchelonRendererWindowDrv
		PRIVATE $<$<CONFIG:Release>:RELEASE_CONFIG>
	)
	add_definitions(/ZI)
	SET_TARGET_PROPERTIES(DeusExEchelonRendererWindowDrv PROPERTIES LINK_FLAGS "/SAFESEH:NO")
endif(MSVC)

target_precompile_headers(DeusExEchelonRendererWindowDrv
	PRIVATE
	"DeusExEchelonRendererWindowDrv_PCH.h"
)

#---------------------------------------------------------------------------------------------------
# Configure IDE
source_group(TREE "${CMAKE_SOURCE_DIR}/Source/WindowDrv" PREFIX "Source Files" FILES ${SRC_BASE})
source_group(TREE "${CMAKE_SOURCE_DIR}/Source/WindowDrv" PREFIX "Header Files" FILES ${HDR_BASE})

#---------------------------------------------------------------------------------------------------
# Configure INSTALL target
install(DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" DESTINATION "${EE_DEUSEXFOLDER}")
install(FILES "${CMAKE_SOURCE_DIR}/Source/WindowDrv/${DEUSEX_INT_FILE}" DESTINATION 	"${EE_DEUSEXFOLDER}")
install(FILES "${CMAKE_SOURCE_DIR}/EXTERNAL/${SDL2_VERSION}/lib/x86/SDL2.dll" DESTINATION 	"${EE_DEUSEXFOLDER}")
