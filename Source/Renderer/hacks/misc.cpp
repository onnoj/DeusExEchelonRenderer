#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include <MurmurHash3.cpp>
#include <xxhash.c>
#include <TlHelp32.h> // Need to include this for CreateToolhelp32Snapshot

#include <Psapi.h>
#pragma comment(lib, "Psapi.lib") //TODO: Move to CMake

//////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <algorithm>
#include <string>
#include <deque>
#include <sstream>
#include <thread>
#include <mutex>

#include "misc.h"

#include "uefacade.h"
#include "utils/memory.h"
#include "utils/debugmenu.h"

//////////////////////////////////////////////////////////////////////////
FrameContextManager g_ContextManager;

GlobalRenderOptions g_options;
namespace Misc
{
  UD3D9FPRenderDevice* g_Facade = nullptr;
}

bool Misc::IsNvRemixAttached(bool pReevaluate)
{
  const auto func = []() {
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
      return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe32)) {
      do {
        if (pe32.th32ParentProcessID == GetProcessId(hProcess)) {
          std::wstring processName(pe32.szExeFile);
          if (processName == L"NvRemixBridge.exe") {
            CloseHandle(hSnap);
            return true;
          }
        }
      } while (Process32Next(hSnap, &pe32));
    }

    CloseHandle(hSnap);
    return false;
  };

  static bool flag = func();
  if (pReevaluate)
    flag = func();
  return flag;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Misc
{
  float rangeFromPlayer(FBspNode* pNode)
  {
    auto ctx = g_ContextManager.GetContext();
    if (ctx == nullptr)
    {
      return FLT_MAX;
    }
    UModel* Model = ctx->frameSceneNode->Level->Model;
    auto& GVerts = Model->Verts;
    auto& GVertPoints = Model->Points;

    FBox bounds(0);
    for (int i = 0; i < pNode->NumVertices; i++)
    {
      bounds += GVertPoints(GVerts(pNode->iVertPool + i).pVertex);
    }
    const float distanceFromPlayer = min(
      (ctx->frameSceneNode->Coords.Origin - bounds.Min).Size(),
      (ctx->frameSceneNode->Coords.Origin - bounds.Max).Size()
    );
    return distanceFromPlayer;
  }
}

//////////////////////////////////////////////////////////////////////////

std::deque<FrameContextManager::Context> FrameContextManager::m_stack;

void FrameContextManager::PushFrameContext()
{
  auto currentContext = GetContext();
  assert(m_stack.size() < 50);
  m_stack.push_back(currentContext ? FrameContextManager::Context(*currentContext) : FrameContextManager::Context{});
}

void FrameContextManager::PopFrameContext()
{
  m_stack.pop_back();
}

FrameContextManager::Context* FrameContextManager::GetContext()
{
  if (!m_stack.empty())
  {
    return &(m_stack.back());
  }
  return nullptr;
}

//////////////////////////////////////////////////////////////////////////

D3DXMATRIX UECoordsToMatrix(const FCoords& pCoords, D3DXMATRIX* pInverse/*=nullptr*/)
{
  static const D3DXMATRIX identity = []() {D3DXMATRIX m; D3DXMatrixIdentity(&m); return m; }();

  D3DXMATRIX correction = identity;
#if defined(CONVERT_TO_LEFTHANDED_COORDINATES) && CONVERT_TO_LEFTHANDED_COORDINATES==1
  correction(0, 0) *= -1.0f;
#endif

  FVector axis[3]{ FVector(1.0f,0.0f,0.0f),FVector(0.0f, 1.0f, 0.0f),FVector(0.0f, 0.0f, 1.0f) };
  axis[0] = axis[0].TransformVectorBy(pCoords);
  axis[1] = axis[1].TransformVectorBy(pCoords);
  axis[2] = axis[2].TransformVectorBy(pCoords);

  D3DXMATRIX rotationTranslationM = identity;
  rotationTranslationM(0, 0) = axis[0].X;
  rotationTranslationM(0, 1) = axis[0].Y;
  rotationTranslationM(0, 2) = axis[0].Z;

  rotationTranslationM(1, 0) = axis[1].X;
  rotationTranslationM(1, 1) = axis[1].Y;
  rotationTranslationM(1, 2) = axis[1].Z;

  rotationTranslationM(2, 0) = axis[2].X;
  rotationTranslationM(2, 1) = axis[2].Y;
  rotationTranslationM(2, 2) = axis[2].Z;

  FVector translation = FVector(0.0f, 0.0f, 0.0f);
  translation = translation.TransformPointBy(pCoords);
  rotationTranslationM(3, 0) = translation.X;
  rotationTranslationM(3, 1) = translation.Y;
  rotationTranslationM(3, 2) = translation.Z;


  D3DXMATRIX result;
  D3DXMatrixMultiply(&result, &rotationTranslationM, &correction);
  if (pInverse != nullptr)
  {
    D3DXMatrixInverse(pInverse, nullptr, &result);
  }
  return result;
}

//////////////////////////////////////////////////////////////////////////

#include "warningscreen.inc"