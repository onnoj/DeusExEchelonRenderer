#Configure project settings:
add_definitions(-D_DLL -DEE_LIBDLL -DDEUSEX=1) 

set(DEUSEX_INT_FILE "DeusExEchelonRenderer.int")
set(DEUSEX_RTX_FILE "rtx.conf")

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

set(SRC_BASE
	"uefacade.cpp"
	"rendering/llrenderer.cpp"
	"rendering/hlrenderer.cpp"
	"rendering/dxtexture.cpp"
	"rendering/dxtexturemanager.cpp"
	"rendering/lightmanager.cpp"
	"utils/debugmenu.cpp"
	"utils/memory.cpp"
	"utils/materialdebugger.cpp"
	"hacks/misc.cpp"
	"hacks/urender.cpp"
	"hacks/uengine.cpp"
	"hacks/fdynamicitemfilter.cpp"
	"hacks/fscenenode.cpp"
	"hacks/fspanbuffer.cpp"
	"hacks/threadaffinity.cpp"
	"hacks/bytepatches.cpp"
	"hacks/rtxconf.cpp"
	"ForceLinkerReferences.cxx"
)

set(HDR_BASE
	"DeusExEchelonRenderer_PCH.h"
	"uefacade.h"
	"rendering/llrenderer.h"
	"rendering/hlrenderer.h"
	"rendering/dxtexture.h"
	"rendering/dxtexturemanager.h"
	"rendering/lightmanager.h"
	"utils/debugmenu.h"
	"utils/memory.h"
	"utils/materialdebugger.h"
	"hacks/misc.h"
	"hacks/hacks.h"
	"${DEUSEX_INT_FILE}"
	"${DEUSEX_RTX_FILE}"
)
add_library (DeusExEchelonRenderer SHARED ${SRC_BASE} ${HDR_BASE})

#source_group("Header Files" FILES ${HDR_BASE})
source_group(TREE "${CMAKE_SOURCE_DIR}/Source" PREFIX "Source Files" FILES ${SRC_BASE})
source_group(TREE "${CMAKE_SOURCE_DIR}/Source" PREFIX "Header Files" FILES ${HDR_BASE})

set_source_files_properties(${HDR_BASE} PROPERTIES HEADER_FILE_ONLY 1)
find_library(LIB_DEUSEX "deusex.lib")
find_library(LIB_CORE "core.lib")
find_library(LIB_ENGINE "engine.lib")
find_library(LIB_RENDER "render.lib")
find_library(LIB_D3D9 "d3d9.lib")
find_library(LIB_D3DX9 "d3dx9.lib")
find_library(LIB_SDL2 "sdl2.lib")

set(DEUSEX_LIBS 
	"${LIB_DEUSEX}" "${LIB_CORE}" "${LIB_ENGINE}" "${LIB_RENDER}"
	"winmm.lib" "comctl32.lib"
	"${LIB_D3D9}" "${LIB_D3DX9}" "${LIB_SDL2}"
	)
target_link_libraries (DeusExEchelonRenderer ${DEUSEX_LIBS})
#target_link_libraries (DeusExEchelonRenderer debug ${LIB_DEBUG_Effects11} OPTIMIZED ${LIB_Effects11})

find_package(fmt)
target_link_libraries(DeusExEchelonRenderer fmt::fmt PolyHook_2)



if(MSVC)
	add_definitions(-D_UNICODE -DUNICODE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t- /Zp4 /Zc:inline- ") #/Zp4 because Deus Ex requires it, /Zc:inline- /GR for debugging purposes.
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI /GR")
	deusex_replace_cmake_cxx_flags(/Zi /ZI)
	SET_TARGET_PROPERTIES(DeusExEchelonRenderer PROPERTIES VS_DEBUGGER_COMMAND "${EE_DEUSEXFOLDER}/deusex.exe")
	SET_TARGET_PROPERTIES(DeusExEchelonRenderer PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${EE_DEUSEXFOLDER}")
	SET_TARGET_PROPERTIES(DeusExEchelonRenderer PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /DEBUG:FULL /OPT:NOICF /OPT:NOREF /DYNAMICBASE:NO /LIBPATH:\"${CMAKE_LIBRARY_PATH}\"")
	set_target_properties(DeusExEchelonRenderer PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}>)
	target_compile_definitions(DeusExEchelonRenderer
		PRIVATE $<$<CONFIG:Release>:RELEASE_CONFIG>
	)
	add_definitions(/ZI)
	SET_TARGET_PROPERTIES(DeusExEchelonRenderer PROPERTIES LINK_FLAGS "/SAFESEH:NO")
endif(MSVC)

target_precompile_headers(DeusExEchelonRenderer
	PRIVATE
	"DeusExEchelonRenderer_PCH.h"
)
install(FILES "${CMAKE_SOURCE_DIR}/Source/${DEUSEX_INT_FILE}" DESTINATION 	"${EE_DEUSEXFOLDER}")
install(FILES "${CMAKE_SOURCE_DIR}/Source/${DEUSEX_RTX_FILE}" DESTINATION 	"${EE_DEUSEXFOLDER}")
install(FILES "${CMAKE_SOURCE_DIR}/EXTERNAL/${SDL2_VERSION}/lib/x86/SDL2.dll" DESTINATION 	"${EE_DEUSEXFOLDER}")