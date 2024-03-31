/*=============================================================================
	Engine.h: Unreal engine public header file.
	Copyright 1997-1999 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef _INC_ENGINE
#define _INC_ENGINE

/*----------------------------------------------------------------------------
	API.
----------------------------------------------------------------------------*/

#ifndef ENGINE_API
	#define ENGINE_API DLL_IMPORT
#endif

/*-----------------------------------------------------------------------------
	Dependencies.
-----------------------------------------------------------------------------*/

#include "Core.h"

/*-----------------------------------------------------------------------------
	Global variables.
-----------------------------------------------------------------------------*/

ENGINE_API extern class FMemStack	GEngineMem;
ENGINE_API extern class FMemCache	GCache;

/*-----------------------------------------------------------------------------
	Engine compiler specific includes.
-----------------------------------------------------------------------------*/

#if __GNUG__
	#include "UnEngineGnuG.h"
#endif

/*-----------------------------------------------------------------------------
	Engine public includes.
-----------------------------------------------------------------------------*/

#include "UnObj.h"				// Standard object definitions.
#include "UnPrim.h"				// Primitive class.
#include "UnModel.h"			// Model class.
#include "UnTex.h"				// Texture and palette.
#include "EngineClasses.h"		// All actor classes.
#include "UnReach.h"			// Reach specs.
#include "UnURL.h"				// Uniform resource locators.
#include "UnLevel.h"			// Level object.
#include "UnIn.h"				// Input system.
#include "UnPlayer.h"			// Player class.
#include "UnEngine.h"			// Unreal engine.
#include "UnGame.h"				// Unreal game engine.
#include "UnCamera.h"			// Viewport subsystem.
#include "UnMesh.h"				// Mesh objects.
#include "UnActor.h"			// Actor inlines.
#include "UnAudio.h"			// Audio code.
#include "UnDynBsp.h"			// Dynamic Bsp objects.
#include "UnScrTex.h"			// Scripted textures.
#include "UnRenderIterator.h"	// Enhanced Actor Render Interface

// DEUS_EX STM
#include "UnEventManager.h"   // AI event manager.


/*-----------------------------------------------------------------------------
	The End.
-----------------------------------------------------------------------------*/
#endif