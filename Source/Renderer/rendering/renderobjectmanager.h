#pragma once

#include <unordered_map>

using RenderObjectKey = uint32_t;

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
  template <typename TFormat>
  VertexBuffer<TFormat>* AcquireBuffer();

  void Reset();

  VertexBufferI* GetBuffer() { return m_Buffer.get(); }
  const VertexBufferI* GetBuffer() const { return m_Buffer.get(); }

  bool IsClean() const { return m_IsClean;  }

  FTextureInfo& GetTextureInfo(RenderObjectTextureType pTextureType) { return m_textureInfo[static_cast<uint32_t>(pTextureType)]; }
  const FTextureInfo& GetTextureInfo(RenderObjectTextureType pTextureType) const { return m_textureInfo[static_cast<uint32_t>(pTextureType)]; }
  void SetTexture(RenderObjectTextureType pTextureType, const FTextureInfo& pValue) { m_textureInfo[static_cast<uint32_t>(pTextureType)] = pValue; }
  UnrealPolyFlags GetFlags() const { return m_flags; }
  void SetFlags(const UnrealPolyFlags& pFlags) { m_flags = pFlags; }
  FSceneNode* GetSceneNode () { return m_sceneNode.get(); }
  const FSceneNode* GetSceneNode () const { return m_sceneNode.get(); }
  void SetSceneNode(std::unique_ptr<FSceneNode>&& pSceneNode) { m_sceneNode = std::move(pSceneNode); }
private:
  FTextureInfo m_textureInfo[RenderObjectTextureTypeMAX]{};
  UnrealPolyFlags m_flags = 0;
  uint32_t m_primitiveCount = 0;
  std::unique_ptr<FSceneNode> m_sceneNode;
  std::shared_ptr<VertexBufferI> m_Buffer;
  bool m_IsClean = true;
};

template <typename TFormat>
VertexBuffer<TFormat>* RenderObject::AcquireBuffer()
{ 
  if (!m_Buffer)
  {
    m_Buffer = std::make_shared<VertexBuffer<TFormat>>();
  }
  m_IsClean = false;
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
  std::pair<std::shared_ptr<RenderObject>, bool/*isNew*/> AcquireRenderObject(RenderObjectKey pKey, RenderObjectLifetime pLifetime);
  void ResetRenderObjects(RenderObjectLifetime pLifetime);
  //void DeleteRenderObject(std::shared_ptr<RenderObject*>& pmRenderObject);
};