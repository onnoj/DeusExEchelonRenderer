/*=============================================================================
	OpenGlDrv.h: Unreal OpenGL support header.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.

	Revision history:
		* Created by Tim Sweeney
		* Multitexture and context support - Andy Hanson (hanson@3dfx.com) and
		  Jack Mathews (jack@3dfx.com)
		* Unified by Daniel Vogel
		
=============================================================================*/

/*-----------------------------------------------------------------------------
	Includes.
-----------------------------------------------------------------------------*/

#ifdef WIN32
#include <windows.h>
#else
#include <SDL/SDL.h>
#endif
#include <GL/gl.h>
#ifdef WIN32
#include "glext.h"
#else
#include <GL/glext.h>
#endif
#include "Engine.h"
#include "UnRender.h"

/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
