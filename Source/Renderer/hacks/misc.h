#pragma once
#include <optional>
#include <deque>
#include <string>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <d3dx9math.h>
#include "rendering/dxtexturemanager.h"
#include <Psapi.h>
//#include "uefacade.h"

extern uint8_t g_RemixWarningImage[240000];

struct GlobalRenderOptions
{
  bool backwardsOcclusionPass = true;
  bool hasLights = true;
  bool hasObjectMeshes = true;
  bool renderingDisabled = false;
  bool cameraTest = false;
  bool fixThreadAffinity = true;
  bool galaxyMallocFix = true;
  bool hasDebugMenu = true;
  bool clusterNodes = true;
  bool clusterNodesWithSameParent = true;
  bool hasDebugDraw = true;
  bool enableViewportXYOffsetWorkaround = true;
};

extern GlobalRenderOptions g_options;
////////////////////////////////////////////////////////////////////////////////////
static bool EE_HAS_IMGUI()
{
  static bool hasImgui = []() {
    std::string cmdline = ::GetCommandLineA();
    if (cmdline.find("noimgui", 0) != std::string::npos)
    {
      return false;
    }
    if (cmdline.find("remixdev", 0) != std::string::npos || IsDebuggerPresent())
    {
      return true;
    }
#if defined(RELEASE_CONFIG)
    return false;
#else
    return true;
#endif
    }();
    return hasImgui && g_options.hasDebugMenu;
}

class UD3D9FPRenderDevice;
namespace Misc /*should probably rename to util*/
{
  extern UD3D9FPRenderDevice* g_Facade;

  static void setRendererFacade(UD3D9FPRenderDevice* renderer)
  {
    g_Facade = renderer;
  }

  int getFov(int defaultFOV, int resX, int resY);
  extern bool IsNvRemixAttached(bool pReevaluate);
  //extern void patch_deusex();
  extern void setRendererFacade(UD3D9FPRenderDevice* renderer);
  extern float rangeFromPlayer(FBspNode* pNode);

  struct DebugContext
  {
    FSceneNode* m_Frame = nullptr;
  };

  enum class DeusExDLLs
  {
    Engine,
    Core,
    Render
  };

  template <DeusExDLLs TDllType>
  bool IsInDLL(void* pAddress, uint64_t baseOffsetMin = 0, uint64_t baseOffsetMax = 0)
  {
    static HMODULE baseAddress = []() {
      if constexpr (TDllType == DeusExDLLs::Engine)
      {
        return GetModuleHandle(L"engine.dll");
      }
      else if constexpr (TDllType == DeusExDLLs::Core)
      {
        return GetModuleHandle(L"core.dll");
      }
      else if constexpr (TDllType == DeusExDLLs::Render)
      {
        return GetModuleHandle(L"render.dll");
      }
      else
      {
        static_assert(false);
      }
      }();
      static uint64_t moduleLength = [](HMODULE hModule) -> uint64_t {
        MODULEINFO moduleInfo;
        if (GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(moduleInfo))) {
          return uint64_t(moduleInfo.SizeOfImage);
        }
        return uint64_t(0);
        }(baseAddress);

        static uint64_t moduleStart = reinterpret_cast<uint64_t>(baseAddress);
        static uint64_t moduleEnd = reinterpret_cast<uint64_t>(baseAddress) + moduleLength;
        return (reinterpret_cast<uint64_t>(pAddress) >= (moduleStart + baseOffsetMin) &&
          reinterpret_cast<uint64_t>(pAddress) <= (moduleStart + baseOffsetMax) &&
          reinterpret_cast<uint64_t>(pAddress) < moduleEnd);
  }
}

extern D3DXMATRIX UECoordsToMatrix(const FCoords& pCoords, D3DXMATRIX* pInverse = nullptr);
