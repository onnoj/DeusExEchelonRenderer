#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop


#include <winternl.h>
#pragma comment(lib, "ntdll.lib") //TODO: move to cmake
#include <TlHelp32.h> // Need to include this for CreateToolhelp32Snapshot
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

#include "misc.h"
#include "hacks.h"
#include "utils/debugmenu.h"

#include <polyhook2/Detour/NatDetour.hpp>
#include <polyhook2/Virtuals/VFuncSwapHook.hpp>

namespace
{
  void applyThreadAffinityMask(const DWORD tid, const uint32_t pAddress)
  {
    if (!g_options.fixThreadAffinity)
    {
      return;
    }

    HMODULE handles[] = {
      GetModuleHandle(NULL),
      GetModuleHandle(L"ConSys.dll"),
      GetModuleHandle(L"Core.dll"),
      GetModuleHandle(L"DeusEx.dll"),
      GetModuleHandle(L"DeusExText.dll"),
      GetModuleHandle(L"Editor.dll"),
      GetModuleHandle(L"Engine.dll"),
      GetModuleHandle(L"Extension.dll"),
      GetModuleHandle(L"Fire.dll"),
      GetModuleHandle(L"Galaxy.dll"),
      GetModuleHandle(L"IpDrv.dll"),
      GetModuleHandle(L"Render.dll"),
      GetModuleHandle(L"Window.dll"),
    };

    HMODULE m{ 0 };
    if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCSTR>(pAddress), &m))
    {
      return;
    }
    bool match = false;
    for (int i = 0; i < std::size(handles); i++)
    {
      if (handles[i] == m)
      {
        match = true;
      }
    }

    if (match)
    {
      HANDLE threadHandle = OpenThread(STANDARD_RIGHTS_WRITE | THREAD_SET_LIMITED_INFORMATION, false, tid);
      {
        SetThreadAffinityMask(threadHandle, 1);
      }
      CloseHandle(threadHandle);
    }
  }

  void applyThreadAffinityMasks()
  {
    if (!g_options.fixThreadAffinity)
    {
      return;
    }
    const auto pid = GetCurrentProcessId();
    HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0/*can only fetch system-wide*/);
    if (h != INVALID_HANDLE_VALUE)
    {
      THREADENTRY32 te{};
      te.dwSize = sizeof(te);
      for (bool result = Thread32First(h, &te); result; result = Thread32Next(h, &te))
      {
        if (te.th32OwnerProcessID == pid && te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
        {
          HANDLE threadHandle = OpenThread(READ_CONTROL | THREAD_QUERY_INFORMATION, false, te.th32ThreadID);
          {
            DWORD dwThreadStartAddr = 0;
            NtQueryInformationThread(threadHandle, (THREADINFOCLASS)9, &dwThreadStartAddr, sizeof(PVOID), 0);
            applyThreadAffinityMask(te.th32ThreadID, dwThreadStartAddr);
          }
          CloseHandle(threadHandle);
        }
      }
    }
  }
}

namespace Hacks
{
  std::vector<std::shared_ptr<PLH::IHook>> ThreadAffinityDetours;
  namespace ThreadAffinityFuncs
  {
    HookableFunction CreateThread = &::CreateThread;
  }

  HANDLE _stdcall CreateThreadOverride(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
  {
    DWORD threadId = 0;
    auto ret = ThreadAffinityFuncs::CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, &threadId);
    if (ret != INVALID_HANDLE_VALUE)
    {
      applyThreadAffinityMask(threadId, uint32_t(*lpStartAddress));
    }
    if (lpThreadId != nullptr)
    {
      *lpThreadId = threadId;
    }
    return ret;
  }

  namespace ThreadAffinityOverrides
  {
    auto CreateThread = &CreateThreadOverride;
  }
}

void InstallThreadAffinityHacks()
{
  applyThreadAffinityMasks();

  using namespace Hacks;
  ThreadAffinityDetours.push_back(std::make_shared<PLH::NatDetour>(*(uint64_t*)&ThreadAffinityFuncs::CreateThread, *(uint64_t*)&ThreadAffinityOverrides::CreateThread, &ThreadAffinityFuncs::CreateThread.func64));
  for (auto& detour : ThreadAffinityDetours)
  {
    detour->hook();
  }
}

void UninstallThreadAffinityHacks()
{
  using namespace Hacks;
  for (auto& detour : ThreadAffinityDetours)
  {
    detour->unHook();
  }
  ThreadAffinityDetours.clear();
  ThreadAffinityFuncs::CreateThread.Restore();
}