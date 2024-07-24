#pragma once
#include <d3dx9tex.h>
#include "d3d.h"
#include "dxtexture.h"
#include "llrenderer.h"

#include <unordered_set>

struct TextureHash
{
  uint32_t m_Hash = 0;
  operator uint32_t() const { return m_Hash; }
  static TextureHash FromTextureInfo(FTextureInfo* pTextureInfo, UnrealPolyFlags pFlags);
};

struct TextureSet
{
  std::optional<TextureMetaData> diffuseTexture;
  std::optional<TextureMetaData> lightMapTexture;
  std::optional<TextureMetaData> detailTexture;
  std::optional<TextureMetaData> fogMapTexture;
  std::optional<TextureMetaData> macroTexture;

  bool operator==(const TextureSet& rh) const { 
    return diffuseTexture.value_or(TextureMetaData{}).cacheID == rh.diffuseTexture.value_or(TextureMetaData{}).cacheID &&
      lightMapTexture.value_or(TextureMetaData{}).cacheID == rh.lightMapTexture.value_or(TextureMetaData{}).cacheID &&
      detailTexture.value_or(TextureMetaData{}).cacheID == rh.detailTexture.value_or(TextureMetaData{}).cacheID &&
      fogMapTexture.value_or(TextureMetaData{}).cacheID == rh.fogMapTexture.value_or(TextureMetaData{}).cacheID &&
      macroTexture.value_or(TextureMetaData{}).cacheID == rh.macroTexture.value_or(TextureMetaData{}).cacheID;
  }
  bool operator!=(const TextureSet& rh) const { return !(*this == rh); }
};

class TextureManager
{
public:
  TextureManager() = default;
  virtual ~TextureManager();

  void Initialize(LowlevelRenderer* pLLRenderer);
  void Shutdown();

  void FlushTextures();

  DeusExD3D9TextureHandle ProcessTexture(UnrealPolyFlags pFlags, FTextureInfo* pUETextureInfo);
  void ProcessUETexture(const uint32_t pKey, UnrealPolyFlags pFlags, FTextureInfo* pUETextureInfo, DeusExD3D9TextureHandle& handle);
  void ProcessHijackedTexture(uint32_t pKey, UnrealPolyFlags pFlags, FTextureInfo* pUETextureInfo, DeusExD3D9TextureHandle& handle);
  bool BindTexture(DWORD polygonFlags, const DeusExD3D9TextureHandle& pTextureHandle);
  std::vector<DeusExD3D9TextureHandle> FindTextures(uint32_t pUETextureCacheID);

  DeusExD3D9TextureHandle GetFakeTexture() { return m_FakeTexture; };
private:
  std::unordered_map<uint32_t, DeusExD3D9TextureHandle> m_InstanceCache;
  std::unordered_multimap<uint32_t/*CacheID*/, DeusExD3D9TextureHandle> m_TextureCache;
  
  LowlevelRenderer* m_llrenderer = nullptr;
  DeusExD3D9TextureHandle m_FakeTexture;
  std::unordered_set<std::wstring> m_HijackableTextures;
};