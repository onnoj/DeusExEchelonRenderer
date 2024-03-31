/*=============================================================================
	HookSGL.h: Header for dynamically linking SGL.LIB

	Copyright 1997 NEC Electronics Inc.
	Based on code copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Jayeson Lee-Steere from code by Tim Sweeney
		* 970112 JLS - Started changes to use new hardware interface.

=============================================================================*/

#ifndef HOOKSGL_H
#define HOOKSGL_H

// Selects the correct header depending on what type of build we are doing.
#include "sgl.h"


// Class containing pointers to all SGL Direct functions. This is used to
// dynamically link to the .DLL.
class USGL
{
public:
	// From sgl.h:
#undef API_FN
#define API_FN(RetVal,Name,Arguments) RetVal (CALL_CONV *Name)Arguments;
	
	// Windows routines.
#if WIN32
	API_FN(sgl_win_versions*, sgl_get_win_versions, (void))
	API_FN(int, sgl_use_ddraw_mode,		(void *hWnd, PROC_2D_CALLBACK Proc2d))
	API_FN(int, sgl_use_address_mode,	(PROC_ADDRESS_CALLBACK ProcNextAddress, sgl_uint32 **pStatus))
	API_FN(int, sgl_use_eor_callback,	(PROC_END_OF_RENDER_CALLBACK ProcEOR))
#endif
	// Device routines.
	API_FN(void,	sgl_get_errors, (int *earliest_error, int *most_recent_error ))
	API_FN(int,		sgl_create_screen_device, (int device_number, int x_dimension, int y_dimension, sgl_device_colour_types device_mode, sgl_bool double_buffer))
	API_FN(int,		sgl_get_device, (int device_name, int * device_number, int * x_dimension, int * y_dimension, sgl_device_colour_types * device_mode, sgl_bool * double_buffer))
	API_FN(void,	sgl_delete_device, (int device_name))
	// Texture routines.
	API_FN(int,		sgl_create_texture, (sgl_map_types map_type, sgl_map_sizes map_size, sgl_mipmap_generation_options generate_mipmap, sgl_bool dither, const sgl_intermediate_map *pixel_data, const sgl_intermediate_map *filtered_maps[]))
	API_FN(int,		sgl_preprocess_texture, (sgl_map_types map_type,sgl_map_sizes map_size, sgl_mipmap_generation_options generate_mipmap, sgl_bool dither, const sgl_intermediate_map *src_pixel_data, const sgl_intermediate_map *filtered_maps[],sgl_intermediate_map *processed_map))
	API_FN(long,	sgl_texture_size, (sgl_intermediate_map *texture_map))
	API_FN(void,   	sgl_set_texture, (int texture_name, sgl_mipmap_generation_options generate_mipmap, sgl_bool dither, const sgl_intermediate_map *pixel_data, const sgl_intermediate_map *filtered_maps[]))
	API_FN(void,   	sgl_direct_set_texture, (int TextureName, int MapLevel, sgl_map_types NewMapType, sgl_direct_srctype EnumSrcType, int SrcX, int SrcY, const void *pPixelData, sgl_uint32 StrideInUINT32, const sgl_direct_pixformat_struct *pPixFormat, sgl_bool UseChromaKey, sgl_uint32 ChromaColourOrIndex, const void *Palette ))
//	API_FN(int, 	sgl_set_texture_extended, (int texture_name, sgl_map_types new_map_type, sgl_map_sizes new_map_size, sgl_mipmap_generation_options generate_mipmap, sgl_bool dither, const sgl_intermediate_map *pixel_data, const sgl_intermediate_map *filtered_maps[]))
	API_FN(void,	sgl_delete_texture, (int texture_name))
	API_FN(unsigned long, sgl_get_free_texture_mem, ())
//	API_FN(void,	sgl_get_free_texture_mem_info, (sgl_texture_mem_info *info))
	// Random number generator.
	API_FN(long, 	sgl_rand, (void))
	API_FN(void,	sgl_srand, (unsigned long Seed))
	// Version information.
	API_FN(sgl_versions *, sgl_get_versions, (void))
	// Windows texture extensions.
	API_FN(sgl_intermediate_map, ConvertBMPtoSGL, (char * filename, sgl_bool Translucent))
	API_FN(int,	LoadBMPTexture, (char *pszFilename, sgl_bool bTranslucent, sgl_mipmap_generation_options generate_mipmap, sgl_bool dither))
	API_FN(void,	FreeBMPTexture, (int nTextureName))
	API_FN(void,	FreeAllBMPTextures, (void))
	// sgltri_ functions.
	API_FN(void,  sgltri_startofframe, (PSGLCONTEXT pContext))
	API_FN(void,  sgltri_triangles, (PSGLCONTEXT pContext, int nNumFaces, int pFaces[][3], PSGLVERTEX pVertices ))
	API_FN(void,  sgltri_quads, (PSGLCONTEXT pContext, int nNumFaces, int pFaces[][4], PSGLVERTEX pVertices ))
	API_FN(void,  sgltri_points, (PSGLCONTEXT pContext, int nNumPoints, PSGLVERTEX pVertices ))
	API_FN(void,  sgltri_lines, (PSGLCONTEXT pContext, int nLines, sgl_uint16 pLines[][2], PSGLVERTEX pVertices ))
	API_FN(void,  sgltri_shadow, (PSGLCONTEXT pContext, int nNumFaces, int pFaces[][3], PSGLVERTEX pVertices, float fBoundingBox[2][2]))
	API_FN(void,  sgltri_render, (PSGLCONTEXT pContext))
	API_FN(void,  sgltri_rerender, (PSGLCONTEXT pContext))
	API_FN(IRC_RESULT, sgltri_isrendercomplete, (PSGLCONTEXT pContext, sgl_uint32 u32Timeout))
	// .INI files/registry reading routines.
	API_FN(sgl_bool, sgl_get_ini_string, (char *ReturnData, int ReturnSize, char *DefaultDataValue, char *Section, char *Entry))
	API_FN(sgl_bool, sgl_get_ini_int, (sgl_int32 *ReturnData, sgl_int32 DefaultDataValue, char *Section, char *Entry))
#undef API_FN

	SGLCONTEXT SglContext;

	// Variables and functions.
	HINSTANCE hModule;
	int Ok;
	FARPROC Find(char *Name);
	int HookSGL();
	void UnhookSGL();
	char *GetSGLErrorString(int ErrorValue);
};

#endif // HOOKSGL_H