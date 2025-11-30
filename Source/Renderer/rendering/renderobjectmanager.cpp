#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "renderobjectmanager.h"

std::pair<std::shared_ptr<RenderObject>, bool/*isNew*/> RenderObjectManager::AcquireRenderObject(RenderObjectKey pKey, RenderObjectLifetime pLifetime)
{
  std::shared_ptr<RenderObject> ro;

  if (pLifetime == RenderObjectLifetime::Instant)
  {
    for (auto& it : m_InstantObjects)
    {
      if (it->IsClean())
      {
        return {it, true};
      }
    }
    ro = std::make_shared<RenderObject>();
    m_InstantObjects.push_back(ro);
    return {ro, true};
  }

  auto& map = m_RenderObjectMap[static_cast<uint32_t>(pLifetime)];
  check(pKey != 0);
  bool isNew = false;
  if (auto m = map.find(pKey); m == map.end())
  {
    ro = std::make_shared<RenderObject>();
    map.insert({ pKey, ro });
    isNew = true;
  }
  else {
    ro = m->second;
    isNew = m->second->IsClean();
  }

  return { ro , isNew };
}

void RenderObjectManager::ResetRenderObjects(RenderObjectLifetime pLifetime)
{
  if (pLifetime == RenderObjectLifetime::Instant)
  {
    for (auto& it : m_InstantObjects)
    {
      it->Reset();
    }
    return;
  }

  auto& map = m_RenderObjectMap[static_cast<uint32_t>(pLifetime)];
  for (auto& it : map)
  {
    it.second->Reset();
  }
}

///

void RenderObject::Reset()
{
  m_primitiveCount = 0;
  m_IsClean = true;
  if (m_Buffer)
  {
    m_Buffer->Reset();
  }
}