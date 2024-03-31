#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include <MurmurHash3.cpp>
#include <xxhash.c>
#include <fmt/format.h>
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

#include "warningscreen.inc"