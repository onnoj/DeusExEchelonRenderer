#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop
#include <mutex>
#include "memory.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static constexpr auto defaultAlignment = 4;
FMalloc* g_originalMalloc = nullptr;
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Kept to help debugging weird level transition and galaxy.dll crash issues
// ultimately doesn't seem to be related to this malloc.
class SafeMalloc : public FMalloc
{
private:
  std::recursive_mutex m;
public:
  void* Malloc(DWORD Count, const TCHAR* Tag) final
  {
    m.lock();
    void* ret = g_originalMalloc->Malloc(Count, Tag);
    m.unlock();
    return ret;
  }

  void* Realloc(void* Original, DWORD Count, const TCHAR* Tag) final
  {
    m.lock();
    void* ret = g_originalMalloc->Realloc(Original, Count, Tag);;
    m.unlock();
    return ret;
  }

  void Free(void* Original) final
  {
    m.lock();
    g_originalMalloc->Free(Original);
    m.unlock();
  }

  void DumpAllocs() final
  {
  }

  void HeapCheck() final
  {
  }

  void Init() final
  {
  }

  void Exit() final
  {
  }
} static g_safeMalloc;
//////////////////////////////////////////////////////////////////////////////////////////
class CustomMalloc : public FMalloc
{
private:
  std::recursive_mutex m;
  void* m_trackedAllocations[1000000]{};
public:
  void* Malloc(DWORD Count, const TCHAR* Tag) final
  {
    std::scoped_lock(m);
    void* ret = _malloc_dbg(Count * 2, _NORMAL_BLOCK, nullptr, 0);
    if (ret != nullptr)
    {
      for (auto& e : m_trackedAllocations)
      {
        if (e == nullptr)
        {
          e = ret;
          break;
        }
      }
    }
    return ret;
  }

  void* Realloc(void* Original, DWORD Count, const TCHAR* Tag) final
  {
    std::scoped_lock(m);
    bool isKnownAlloc = false;
    if (Original != nullptr)
    {
      for (auto& e : m_trackedAllocations)
      {
        if (e == Original)
        {
          isKnownAlloc = true;
          e = nullptr;
          break;
        }
      }
    }
    else {
      isKnownAlloc = true;
    }

    if (isKnownAlloc)
    {
      void* ret = ::_realloc_dbg(Original, Count * 2, _NORMAL_BLOCK, nullptr, 0);
      for (auto& e : m_trackedAllocations)
      {
        if (e == nullptr)
        {
          e = ret;
          break;
        }
      }
      return ret;
    }
    else
    {
      void* ret = g_originalMalloc->Realloc(Original, Count, Tag);;
      return ret;
    }
  }

  void Free(void* Original) final
  {
    std::scoped_lock(m);

    if (Original != nullptr)
    {
      for (auto& e : m_trackedAllocations)
      {
        if (e == Original)
        {
          ::_free_dbg(Original, _NORMAL_BLOCK);
          e = nullptr;
          return;
        }
      }

      g_originalMalloc->Free(Original);
    }
  }

  void DumpAllocs() final
  {
  }

  void HeapCheck() final
  {
  }

  void Init() final
  {
  }

  void Exit() final
  {
  }
} static g_TrackedMalloc;
//////////////////////////////////////////////////////////////////////////////////////////
auto dummy = []() {
  g_originalMalloc = GMalloc;
  GMalloc = &g_safeMalloc;
  //GMalloc = &g_TrackedMalloc;
  return 0;
}();

