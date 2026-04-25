#pragma once

#include <unordered_map>
#include "vertexbuffermanager.h"

using RenderObjectKey = uint64_t;
enum class RenderObjectTextureType
{
  Albedo,
  Lightmap,
  MAX,
};
constexpr uint32_t RenderObjectTextureTypeMAX = static_cast<uint32_t>(RenderObjectTextureType::MAX);

class RenderObject
{
public:
  RenderObject() = default;
  virtual ~RenderObject()
  {
    int x = 1;
  }
public:
  template <typename TFormat>
  VertexBuffer<TFormat>* AcquireBuffer();

  void Reset();

  VertexBufferI* GetBuffer() { return m_Buffer.get(); }
  const VertexBufferI* GetBuffer() const { return m_Buffer.get(); }

  bool IsClean() const { return m_IsClean;  }
  bool IsLocked() const { return m_IsLocked; }
  void SetIsLocked(bool pValue) { m_IsLocked = pValue; }

  FTextureInfo& GetTextureInfo(RenderObjectTextureType pTextureType) { return m_textureInfo[static_cast<uint32_t>(pTextureType)]; }
  const FTextureInfo& GetTextureInfo(RenderObjectTextureType pTextureType) const { return m_textureInfo[static_cast<uint32_t>(pTextureType)]; }
  void SetTexture(RenderObjectTextureType pTextureType, const FTextureInfo& pValue) { m_textureInfo[static_cast<uint32_t>(pTextureType)] = pValue; }
  UnrealPolyFlags GetFlags() const { return m_flags; }
  void SetFlags(const UnrealPolyFlags& pFlags) { m_flags = pFlags; }
  FSceneNode* GetSceneNode () { return m_sceneNode.get(); }
  const FSceneNode* GetSceneNode () const { return m_sceneNode.get(); }
  void SetSceneNode(std::unique_ptr<FSceneNode>&& pSceneNode) { m_sceneNode = std::move(pSceneNode); }
private:
  using VertexBufferMap = std::unordered_map<uint32_t, std::shared_ptr<VertexBufferI>>;
  using VertexBufferPair = std::pair<uint32_t/*key*/, std::shared_ptr<VertexBufferI>>;

  FTextureInfo m_textureInfo[RenderObjectTextureTypeMAX]{};
  UnrealPolyFlags m_flags = 0;
  uint32_t m_primitiveCount = 0;
  std::unique_ptr<FSceneNode> m_sceneNode;
  std::shared_ptr<VertexBufferI> m_Buffer;
  bool m_IsClean = true;
  bool m_IsLocked = false;
public:
  DWORD m_Debug = 0;
};

template <typename TFormat>
VertexBuffer<TFormat>* RenderObject::AcquireBuffer()
{ 
  if (!m_Buffer)
  {
    m_Buffer = std::make_shared<VertexBuffer<TFormat>>();
  }
  m_IsClean = false;
  check(m_Buffer->GetFVF() == TFormat::GetFVF());
  return static_cast<VertexBuffer<TFormat>*>(m_Buffer.get());
}

enum class RenderObjectLifetime
{
  Instant,
  Frame,
  Level,
  Application,
  MAX,
};
constexpr uint32_t RenderObjectLifetimeMax = static_cast<uint32_t>(RenderObjectLifetime::MAX);

class RenderObjectManager
{
private:
  std::unordered_map<RenderObjectKey, std::shared_ptr<RenderObject>> m_RenderObjectMap[RenderObjectLifetimeMax];
  std::vector<std::shared_ptr<RenderObject>> m_InstantObjects;
public:
  template <typename TFormat>
  std::pair<std::shared_ptr<RenderObject>, bool/*isNew*/> AcquireRenderObject(RenderObjectKey pKey, RenderObjectLifetime pLifetime)
  {
    if (pLifetime == RenderObjectLifetime::Instant)
    {
      std::shared_ptr<RenderObject> candidate;
      for (const auto& object : m_InstantObjects)
      {
        const auto buffer = object->GetBuffer();

        // Prefer a clean object with a matching vertex format
        if (buffer &&
          buffer->GetFVF() == TFormat::GetFVF() &&
          object->IsClean())
        {
          return { object, true };
        }
        else if (!buffer)
        {
          candidate = object;
        }
      }

      if (candidate)
      {
        return { candidate, true };
      }

      // No reusable object found: create a new one
      auto object = std::make_shared<RenderObject>();
      m_InstantObjects.push_back(object);
      return { object, true };
    }

    auto& map = m_RenderObjectMap[static_cast<uint32_t>(pLifetime)];
    check(pKey != 0);
    pKey = Utils::CalculateKey(pKey, TFormat::GetFVF());

    auto it = map.find(pKey);
    bool isNew = false;
    if (it == map.end())
    {
      auto object = std::make_shared<RenderObject>();
      map.emplace(pKey, object);
      return { object, true };
    }

    return { it->second , it->second->IsClean() };
  }

  void ResetRenderObjects(RenderObjectLifetime pLifetime);
};