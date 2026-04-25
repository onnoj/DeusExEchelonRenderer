#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "renderobjectmanager.h"

void RenderObjectManager::ResetRenderObjects(RenderObjectLifetime pLifetime)
{
  if (pLifetime == RenderObjectLifetime::Instant)
  {
    for (auto& it : m_InstantObjects)
    {
      if (!it->IsClean())
      {
        it->Reset();
      }
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
  m_IsLocked = false;
  m_flags = 0;
  m_sceneNode = nullptr;
  m_Debug = 0;
  for (auto& texInfo : m_textureInfo)
  {
    texInfo = {};
  }

  if (m_Buffer)
  {
    m_Buffer->Reset();
  }
}