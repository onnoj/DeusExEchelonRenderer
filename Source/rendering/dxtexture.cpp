#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "rendering/dxtexture.h"

void DeusExD3D9Texture::ConvertFrom8bpp(FTextureInfo* pUETextureInfo, UnrealPolyFlags pFlags)
{
  const auto flags = pFlags | pUETextureInfo->Texture->PolyFlags;
  const bool isMasked = (flags & PF_Masked) != 0;

  //We _only_ process mip0, we're not really interested in the other mips for remix...
  auto textureMip0 = pUETextureInfo->Mips[0];

  const auto length = textureMip0->USize * textureMip0->VSize;
  buffer.emplace().resize(length * 4/*channels*/);
  uint8_t* src = reinterpret_cast<uint8_t*>(textureMip0->DataPtr);
  uint8_t* dst = reinterpret_cast<uint8_t*>(buffer->data());
  for (uint32_t i = 0; i < length; i++)
  {
    const auto palleteIndex = src[i];
    static uint8_t blackBytes[] = { 0,0,0,0 };
    const uint8_t* colorBytes = (palleteIndex == 0 && isMasked) ? &blackBytes[0] : reinterpret_cast<uint8_t*>(&pUETextureInfo->Palette[palleteIndex]);

    //convert to a8r8g8b8, little-endian
    const auto b = colorBytes[0];
    const auto g = colorBytes[1];
    const auto r = colorBytes[2];
    const auto a = colorBytes[3];
    dst[(i * 4) + 0] = r;
    dst[(i * 4) + 1] = g;
    dst[(i * 4) + 2] = b;
    dst[(i * 4) + 3] = a;
  }

  this->textureDataPtr = this->buffer->data();
  this->textureDataPitch = textureMip0->USize * 4/*channels*/;
}