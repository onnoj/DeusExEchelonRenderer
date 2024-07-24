#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#pragma comment(lib, "d3dx9.lib")

#include <Core/hooks/hooks.h>

#include "utils/configmanager.h"
#include "utils/utils.h"
#include "rendering/dxtexturemanager.h"

TextureManager::~TextureManager()
{
  m_llrenderer = nullptr;
}

void TextureManager::Initialize(LowlevelRenderer* pLLRenderer)
{
  m_llrenderer = pLLRenderer;
  m_FakeTexture = std::make_unique<DeusExD3D9Texture>();
  m_FakeTexture->format = D3DFMT_DXT1;
  m_FakeTexture->md.cacheID = 0xFFFFFFFFFFFFFFFFull;
  m_FakeTexture->md.width = 800;
  m_FakeTexture->md.height = 600;
  m_FakeTexture->md.multU = 1.0f;
  m_FakeTexture->md.multV = 1.0f;
  m_FakeTexture->textureDataPtr = reinterpret_cast<void*>(&g_RemixWarningImage[0]);
  m_FakeTexture->textureDataPitch = std::size(g_RemixWarningImage) / m_FakeTexture->md.height;
  if (m_llrenderer->AllocateTexture(m_FakeTexture))
  {
    m_FakeTexture->valid = true;
  }
  else
  {
    m_FakeTexture.reset();
  }

  for (const auto& textureName : g_ConfigManager.GetHijackedTextureNames())
  {
    m_HijackableTextures.insert(textureName);
  }

  Hooks::UGameEngineCallbacks::OnNotifyLevelChange.insert(std::make_pair(this, [&]() { FlushTextures(); }));
}

void TextureManager::Shutdown()
{
  m_llrenderer = nullptr;
  Hooks::UGameEngineCallbacks::OnNotifyLevelChange.erase(this);
}

void TextureManager::FlushTextures()
{
  for (auto& p : m_TextureCache)
  {
    p.second->textureD3D9->Release();
  }
  m_TextureCache.clear();

  for (auto& p : m_InstanceCache)
  {
    p.second->textureD3D9->Release();
  }
  m_InstanceCache.clear();
}

DeusExD3D9TextureHandle TextureManager::ProcessTexture(UnrealPolyFlags pFlags, FTextureInfo* pUETextureInfo)
{
  auto key = TextureHash::FromTextureInfo(pUETextureInfo, pFlags);
  if (auto it = m_InstanceCache.find(key); it != m_InstanceCache.end())
  {
    return it->second;
  }

  DeusExD3D9TextureHandle& handle = m_InstanceCache.emplace(std::make_pair(key, std::make_unique<DeusExD3D9Texture>())).first->second;
  handle->valid = false;

  std::wstring textureName = pUETextureInfo->Texture->GetPathName();
  const bool isHijacked = (m_HijackableTextures.find(textureName) != m_HijackableTextures.end());
  if (isHijacked)
  {
    ProcessHijackedTexture(key, pFlags, pUETextureInfo, handle);
  }
  else
  {
    ProcessUETexture(key, pFlags, pUETextureInfo, handle);
  }

  assert(pUETextureInfo->USize == pUETextureInfo->Mips[0]->USize);
  assert(pUETextureInfo->VSize == pUETextureInfo->Mips[0]->VSize);
  const bool isDynamic = (pUETextureInfo->bRealtimeChanged || pUETextureInfo->bRealtime || pUETextureInfo->bParametric);
  //if (isDynamic)
  //{
  //  return handle;
  //}
  //assert(!isDynamic);

  if (m_llrenderer->AllocateTexture(handle))
  {
    handle->valid = true;
    m_TextureCache.insert(std::make_pair(pUETextureInfo->CacheID, handle));
  }
  else
  {
    handle->valid = false;
  }

  //Keep texture data, debug menu might need it.
  if (!g_options.hasDebugMenu)
  {
    if (handle->buffer) handle->buffer->clear();
    handle->buffer.reset();
  }

  return handle;
}

void TextureManager::ProcessUETexture(const uint32_t pKey, UnrealPolyFlags pFlags, FTextureInfo* pUETextureInfo, DeusExD3D9TextureHandle& handle)
{
  //We _only_ process mip0, we're not really interested in the other mips for remix...
  auto textureMip0 = pUETextureInfo->Mips[0];
  assert(pUETextureInfo->UClamp != 0);
  assert(pUETextureInfo->VClamp != 0);
  handle->md.width = pUETextureInfo->UClamp; //should be USize clamped by UClamp if non-0?
  handle->md.height = pUETextureInfo->VClamp; //should be VSize clamped by VClamp if non-0?
  handle->md.multU = 1.0 / (pUETextureInfo->UScale * pUETextureInfo->UClamp);
  handle->md.multV = 1.0 / (pUETextureInfo->VScale * pUETextureInfo->VClamp);
  handle->md.cacheID = pKey;

  switch (pUETextureInfo->Format)
  {
  case TEXF_P8:
  {
    handle->format = D3DFMT_A8R8G8B8;
    handle->textureDataPtr = nullptr; //set by ConvertFrom8bpp
    handle->textureDataPitch = 0; //set by ConvertFrom8bpp
    handle->ConvertFrom8bpp(pUETextureInfo, pFlags);
  }; break;
  case TEXF_RGBA7:
  {
    handle->format = D3DFMT_A8R8G8B8; //Can I assign rgba7 to rgba8?
    handle->textureDataPtr = textureMip0->DataPtr;
    handle->textureDataPitch = textureMip0->USize * sizeof(uint8_t) * 4;
  }; break;
  case TEXF_DXT1:
  {
    const auto blockSize = 4;
    const auto pixelsPerBlock = 8;
    handle->format = D3DFMT_DXT1;
    handle->textureDataPtr = textureMip0->DataPtr;
    handle->textureDataPitch = textureMip0->USize / 2;//max(1, ((textureMip0->USize * pixelsPerBlock) / blockSize));
    handle->md.width += pUETextureInfo->USize % blockSize;
    handle->md.height += pUETextureInfo->VSize % blockSize;
  } break;
  case TEXF_RGBA8:
  {
    handle->format = D3DFMT_A8R8G8B8;
    handle->textureDataPtr = textureMip0->DataPtr;
    handle->textureDataPitch = textureMip0->USize * sizeof(uint8_t) * 4;
  } break;
  case TEXF_RGB8: [[fallthrough]];
  case TEXF_RGB16: [[fallthrough]];
  default:
  {
    assert(false); //unsupported
  } break;
  };
}

void TextureManager::ProcessHijackedTexture(uint32_t pKey, UnrealPolyFlags pFlags, FTextureInfo* pUETextureInfo, DeusExD3D9TextureHandle& handle)
{
  ProcessUETexture(pKey, pFlags, pUETextureInfo, handle);

#if 0
  auto textureMip0 = pUETextureInfo->Mips[0];
  assert(pUETextureInfo->UClamp != 0);
  assert(pUETextureInfo->VClamp != 0);
  handle->md.width = 16;
  handle->md.height = 16;
  handle->md.multU = 1.0 / (pUETextureInfo->UScale * pUETextureInfo->UClamp);
  handle->md.multV = 1.0 / (pUETextureInfo->VScale * pUETextureInfo->VClamp);
  handle->md.cacheID = pKey;
  handle->format = D3DFMT_A8R8G8B8;
  handle->buffer.emplace();
  const auto bufferSize = handle->md.width * handle->md.height * 4/*D3DFMT_A8R8G8B8 format*/;
  handle->buffer->reserve(bufferSize);
  ::memset(handle->buffer->data(), 0, handle->buffer->size());
  handle->textureDataPtr = handle->buffer->data();
  handle->textureDataPitch = handle->md.width * 4/*D3DFMT_A8R8G8B8 format*/;
#endif

  uint32_t key = 0;
  std::wstring textureName = pUETextureInfo->Texture->GetPathName();
  MurmurHash3_x86_32(textureName.c_str(), textureName.size() * sizeof(wchar_t), key, &key);
  MurmurHash3_x86_32(&pFlags, sizeof(pFlags), key, &key);

  //Purposefully corrupt the last 4 bytes of the texture with the hash of this texture variant
  const auto bufferSize = handle->md.height * handle->textureDataPitch;
  uint8_t* buffer = handle->buffer->data();
  uint8_t* lastFourBytes = (buffer + bufferSize) - 4;
  *reinterpret_cast<uint32_t*>(lastFourBytes) = key;
}

bool TextureManager::BindTexture(DWORD polygonFlags, const DeusExD3D9TextureHandle& pTextureHandle)
{
  if (m_llrenderer->SetTextureOnDevice(pTextureHandle.get()))
  {
    if (!(polygonFlags & (PF_Translucent | PF_Modulated | PF_Highlighted))) {
      polygonFlags |= PF_Occlude;
    }
    else if (polygonFlags & PF_Translucent) {
      polygonFlags &= ~PF_Masked;
    }
    if (polygonFlags & PF_Mirrored) {
      polygonFlags &= ~PF_Mirrored;
      polygonFlags &= ~PF_Translucent;
      polygonFlags &= ~PF_Invisible;
    }

    m_llrenderer->ConfigureBlendState(polygonFlags);
    m_llrenderer->ConfigureTextureStageState(0, polygonFlags);
    m_llrenderer->ConfigureSamplerState(0, polygonFlags);
    return true;
  }
  return false;
}

std::vector<DeusExD3D9TextureHandle> TextureManager::FindTextures(uint32_t pUETextureCacheID)
{
  std::vector<DeusExD3D9TextureHandle> handles;
  for (auto it = m_TextureCache.find(pUETextureCacheID); it != m_TextureCache.end(); it++)
  {
    if (it->first == pUETextureCacheID)
    {
      handles.push_back((*it).second);
      continue;
    }
    break;
  }
  return handles;
}

TextureHash TextureHash::FromTextureInfo(FTextureInfo* pTextureInfo, UnrealPolyFlags pFlags)
{
  pFlags |= pTextureInfo->Texture->PolyFlags;

  TextureHash hash{};
  MurmurHash3_x86_32(&pTextureInfo->CacheID, sizeof(pTextureInfo->CacheID), hash.m_Hash, &hash.m_Hash);
  MurmurHash3_x86_32(&pFlags, sizeof(pFlags), hash.m_Hash, &hash.m_Hash);
  return hash;
}