add_definitions(-D_EXE -DEE_EXE -DDEUSEX=1) 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DEV/)

include_directories(AFTER "${CMAKE_SOURCE_DIR}/TOOLS/Source")
include_directories(AFTER "${CMAKE_SOURCE_DIR}/TOOLS/ingestionhelper")

set(SRC_BASE
	"Source/main.cpp"
	"Source/utils.cpp"
	"Source/ingestionhelper.cpp"
)

set(HDR_BASE
	"Source/main.h"
	"Source/utils.h"
	"Source/ingestionhelper.h"
)

add_executable (IngestionHelper ${SRC_BASE} ${HDR_BASE} "IngestionHelper_PCH.h")
target_link_libraries(IngestionHelper scn::scn)

#source_group("Header Files" FILES ${HDR_BASE})
source_group(TREE "${CMAKE_SOURCE_DIR}/TOOLS/ingestionhelper/Source" PREFIX "Source Files" FILES ${SRC_BASE})
source_group(TREE "${CMAKE_SOURCE_DIR}/TOOLS/ingestionhelper/Source" PREFIX "Header Files" FILES ${HDR_BASE})

set_source_files_properties(${HDR_BASE} PROPERTIES HEADER_FILE_ONLY 1)

if(MSVC)
	add_definitions(-D_UNICODE -DUNICODE)
	target_compile_definitions(IngestionHelper
		PRIVATE $<$<CONFIG:Release>:RELEASE_CONFIG>
	)
	set_target_properties(IngestionHelper PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}>)
endif(MSVC)

target_precompile_headers(IngestionHelper
	PRIVATE
	"IngestionHelper_PCH.h"
)
