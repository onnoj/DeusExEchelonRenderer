#pragma once

struct TextureMetaData
{
  uint64_t cacheID = 0;
  float multU = 0.0f;
  float multV = 0.0f;
  uint32_t width = 0;
  uint32_t height = 0;
};

struct DeusExD3D9Texture
{
  TextureMetaData md;
  uint64_t remixHash = 0;
  std::optional<std::vector<uint8_t>> buffer;
  void* textureDataPtr = nullptr; //Can either point to texture data directly, or, to data in the buffer.
  D3DFORMAT format = D3DFMT_UNKNOWN;
  IDirect3DTexture9* textureD3D9 = nullptr;
  uint32_t textureDataPitch = 0;
  bool valid = false;

  void ConvertFrom8bpp(FTextureInfo* pUETextureInfo, UnrealPolyFlags pFlags);
  void ConvertFromRGBA7(FTextureInfo* pUETextureInfo, UnrealPolyFlags pFlags);
};
using DeusExD3D9TextureHandle = std::shared_ptr<DeusExD3D9Texture>;