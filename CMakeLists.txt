cmake_minimum_required (VERSION 3.26.3) #Can probably lower this, but, this is what I tested against :)
project ("DeusExEchelonRenderer")

if(NOT EE_DEUSEXFOLDER)
  set(EE_DEUSEXFOLDER "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Deus Ex")
endif()
string(REPLACE "\\" "/" EE_DEUSEXFOLDER ${EE_DEUSEXFOLDER})

add_definitions(/Zm256 /std:c++17 /fp:precise /MP
  /DEEOS_WINDOWS=1 
  /D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS 
  /D_SILENCE_CXX17_NEGATORS_DEPRECATION_WARNING 
  /DWIN32 
  /D_WIN32_WINNT=0x0A00 
  /D_ITERATOR_DEBUG_LEVEL=0 
  /DWINDOWS_IGNORE_PACKING_MISMATCH
)

if (TRUE)
	add_definitions(/D_SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS)
	MESSAGE("TODO: REPLACE hash_map with unordered_map!")
endif()
  
add_compile_definitions($<$<CONFIG:Debug>:_REALLY_WANT_DEBUG>)
add_compile_definitions($<$<CONFIG:Debug>:EE_DEBUG>)
 
SET(SDL2_VERSION "SDL2-2.26.5")

include_directories(AFTER "${CMAKE_SOURCE_DIR}/INCLUDE/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/INCLUDE/DeusEx/Core/Inc/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/INCLUDE/DeusEx/Render/Inc/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/INCLUDE/DeusEx/Engine/Inc/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/INCLUDE/DeusEx/DeusEx/Inc/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/INCLUDE/DeusEx/System/Inc/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/INCLUDE/DeusEx/Extension/Inc/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/INCLUDE/DeusEx/Window/Inc/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/INCLUDE/DeusEx/WinDrv/Inc/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/INCLUDE/DeusEx/WinDrv/Src/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/EXTERNAL/imgui/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/EXTERNAL/murmurhash/src/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/EXTERNAL/PolyHook_2_0/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/EXTERNAL/PolyHook_2_0/zydis/include/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/EXTERNAL/PolyHook_2_0/zydis/dependencies/zycore/include/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/EXTERNAL/${SDL2_VERSION}/include/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/EXTERNAL/xxHash/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/EXTERNAL/json/include/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/EXTERNAL/CLI11/include/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/EXTERNAL/scnlib/include/")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/Source")

set (CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_SOURCE_DIR}/LIB/")
set (CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_SOURCE_DIR}/LIB/DeusEx/Core/Lib")
set (CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_SOURCE_DIR}/LIB/DeusEx/Render/Lib")
set (CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_SOURCE_DIR}/LIB/DeusEx/Engine/Lib")
set (CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_SOURCE_DIR}/LIB/DeusEx/Extension/Lib")
set (CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_SOURCE_DIR}/LIB/DeusEx/DeusEx/Lib")
set (CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_SOURCE_DIR}/LIB/DeusEx/System/Lib")
set (CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_SOURCE_DIR}/EXTERNAL/${SDL2_VERSION}/lib/x86")
link_directories("${CMAKE_SOURCE_DIR}/LIB/")

#build into BIN folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/BIN/)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/BIN/)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/BIN/)
set_property(GLOBAL PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(CMAKE_FOLDER "EXTERNAL")

set(POLYHOOK_OS "windows")
set(POLYHOOK_BUILD_STATIC_RUNTIME OFF)
add_subdirectory ("EXTERNAL/PolyHook_2_0" EXCLUDE_FROM_ALL)

add_subdirectory ("EXTERNAL/scnlib" EXCLUDE_FROM_ALL)



unset(CMAKE_FOLDER)
add_subdirectory (Source/Core)
add_subdirectory (Source/Renderer)
add_subdirectory (TOOLS/ingestionhelper)



