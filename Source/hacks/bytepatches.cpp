#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "hacks.h"

namespace
{
  void* galaxyMalloc(size_t Size);
  decltype(&galaxyMalloc) originalGalaxyMalloc;
  void* galaxyMalloc(size_t Size)
  {
    return originalGalaxyMalloc(Size*2);
  }
}

void InstallBytePatches()
{
  static bool installed = false;
  if (!installed)
  {
    installed = true;

    //Byte patch: disable backface culling of object meshes
    if (true)
    {
      uint32_t renderModule = reinterpret_cast<uint32_t>(GetModuleHandleA("render.dll"));

      DWORD oldProtect = 0;

      uint32_t offsets[] = { 0xFF95,0xFFC9,0x10317,0x1034B,0xDC81,0xDCBB };
      for (auto offset : offsets)
      {
        if (VirtualProtect(LPVOID(renderModule + offset), 6 * 2, PAGE_READWRITE, &oldProtect))
        {
          for (int i = 0; i < 6; i++)
          {
            *reinterpret_cast<uint8_t*>(uint32_t(renderModule) + offset + i) = 0x90;
          }
          VirtualProtect(LPVOID(renderModule + offset), 6 * 2, oldProtect, &oldProtect);
        }
      }
    }


    //Byte patch: Disable asserts in UObject::UnhashObject
    if (true)
    {
      uint32_t coreModule = reinterpret_cast<uint32_t>(GetModuleHandleA("core.dll"));
      uint32_t offsets[] = { 0x577b7, 0x577d3 };
      DWORD oldProtect = 0;
      for (auto offset : offsets)
      {
        if (VirtualProtect(LPVOID(coreModule + offset), 6 * 2, PAGE_READWRITE, &oldProtect))
        {
          *reinterpret_cast<uint8_t*>(uint32_t(coreModule) + offset + 0) = 0xeb;
          VirtualProtect(LPVOID(coreModule + offset), 6 * 2, oldProtect, &oldProtect);
        }
      }
    }

    if (true)
    {
      constexpr uint32_t newMaxPoints = MAX_POINTS * 2;
      constexpr uint32_t newMaxNodes = MAX_NODES * 2;
      delete(URender::PointCache);
      URender::PointCache = new(URender::FStampedPoint[newMaxPoints]);
      URender::DynamicsCache = new(TEXT("FDynamicsCache"))URender::FDynamicsCache[newMaxNodes];
      appMemzero(URender::DynamicsCache, newMaxNodes * sizeof(URender::FDynamicsCache));
      GDynMem.Init(655360 * 2);
      GSceneMem.Init(327680 * 2);
      URender::VectorMem.Init(163840 * 2);


      uint32_t module = reinterpret_cast<uint32_t>(GetModuleHandleA("render.dll"));
      const uint32_t offsets[] = { 0x1354d + 1, 0x1742e + 6 };
      DWORD oldProtect = 0;
      for (auto offset : offsets)
      {
        if (VirtualProtect(LPVOID(module + offset), 4, PAGE_READWRITE, &oldProtect))
        {
          auto& val = *reinterpret_cast<uint32_t*>(uint32_t(module) + offset + 0);// = 0xeb;
          val = newMaxPoints;
          VirtualProtect(LPVOID(module + offset), 4, oldProtect, &oldProtect);
        }
      }
    }

    //hijack galaxy malloc (non-virtual);
    if (g_options.galaxyMallocFix)
    {
      std::thread([]() {
          do
          {
            uint32_t module = reinterpret_cast<uint32_t>(GetModuleHandleA("galaxy.dll"));
            if (module != 0)
            {
              uint32_t** galaxyMallocPtr = reinterpret_cast<uint32_t**>(module + 0x20D81 + 2);
              uint32_t overridePtr = reinterpret_cast<uint32_t>(&galaxyMalloc);
              originalGalaxyMalloc = reinterpret_cast<decltype(originalGalaxyMalloc)>(**galaxyMallocPtr);
              **galaxyMallocPtr = overridePtr;
              return;
            }
            ::Sleep(1000);
          } while (true);
        }).detach();
    }
  }
}

void UninstallBytePatches()
{
  //nothing to do
}
