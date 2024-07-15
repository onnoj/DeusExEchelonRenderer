#pragma once

#include "DeusExEchelonCore.h"

//Windows
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dxgi.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iomanip> //setprecision
#include <FCNTL.H>
#include <stdio.h>
#include <io.h>
#include <cstdint>
#include <cmath>
#include <Windows.h>
#include <string>
#include <functional>
#include <chrono>
#include <cassert>
#include <MurmurHash3.h>

//Unreal
#pragma pack(push, 4)
#if !defined(DLL_IMPORT)
#define DLL_IMPORT	__declspec(dllimport)
#endif
#include "Core.h"
#include "Engine.h"
#ifndef UNRENDER_H_
#define UNRENDER_H_
//#include "UnRender.h"
#include <DeusEx/Render/Src/RenderPrivate.h>
#endif
//#include "UnRender.h"
#include <deusex/Extension/Inc/ExtensionCore.h>
#include <deusex/Extension/Inc/ExtPlayerPawn.h>
#include <deusex/DeusEx/Inc/DeusEx.h>
#include <deusex/Engine/Inc/UnCon.h>
#pragma pack(pop)
extern UGameEngine* GEngine;

//Undo unreal ifdefs:
#undef M0  
#undef M1  
#undef M2  
#undef M3  
#undef M4  
#undef M5  
#undef M6  
#undef M7  
#undef m0  
#undef m1  
#undef m2  
#undef m3  
#undef m4  
#undef m5  
#undef m6  
#undef m7  
#undef _EAX
#undef _ECX
#undef _EDX
#undef _EBX
#undef _ESI
#undef _EDI
#undef _eax
#undef _ecx
#undef _edx
#undef _ebx
#undef _esi
#undef _edi


#if !defined(NDEBUG)
#undef check
#define check(...) assert(__VA_ARGS__)
#else
#undef check
#define check(...) void(0)
#endif