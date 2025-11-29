#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "vertexbuffermanager.h"

uint32_t VertexBuffer<VertexPos3Tex0>::GetFVF() const { return D3DFVF_XYZ | /*D3DFVF_DIFFUSE |*/ D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/ | D3DFVF_TEXCOORDSIZE2(1); }
uint32_t VertexBuffer<VertexPos3Tex0Tex1>::GetFVF() const { return D3DFVF_XYZ | /*D3DFVF_DIFFUSE | D3DFVF_TEX1 | */ D3DFVF_TEX2 /*| D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5 */ | D3DFVF_TEXCOORDSIZE2(2) | D3DFVF_TEXCOORDSIZE2(1); }
uint32_t VertexBuffer<VertexPos3Norm3Tex0>::GetFVF() const { return D3DFVF_XYZ | D3DFVF_NORMAL | /*D3DFVF_DIFFUSE |*/ D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/ | D3DFVF_TEXCOORDSIZE2(1); }
uint32_t VertexBuffer<VertexPos3Tex0to4>::GetFVF() const { return D3DFVF_XYZ | /*D3DFVF_DIFFUSE |*/ D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/ | D3DFVF_TEXCOORDSIZE2(1); }
uint32_t VertexBuffer<PreTransformedVertexPos4Color0Tex0>::GetFVF() const { return D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/ | D3DFVF_TEXCOORDSIZE2(1); }
uint32_t VertexBuffer<VertexPos4Color0Tex0>::GetFVF() const { return D3DFVF_XYZW | D3DFVF_DIFFUSE | D3DFVF_TEX1 /*| D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/ | D3DFVF_TEXCOORDSIZE2(1); }
uint32_t VertexBuffer<VertexPos3Color0>::GetFVF() const { return D3DFVF_XYZ | D3DFVF_DIFFUSE /*| D3DFVF_TEX1 | D3DFVF_TEX2 | D3DFVF_TEX3 | D3DFVF_TEX4 | D3DFVF_TEX5*/; }

void VertexBufferI::Commit(RenderContext& ctx)
{
  if (!m_Dirty)
  {
    return;
  }
  m_Dirty = false;

  const uint32_t vertexCount = this->GetVertexCount();
  const uint32_t vertexSize = this->GetVertexSize();
  const uint32_t bufferSize = (vertexCount * vertexSize);
  
  if (m_HWVertexBufferSize < bufferSize)
  {
    if (m_HWVertexBuffer != nullptr)
    {
      Release();
    }

    m_HWVertexBuffer = ctx.llRenderer->AllocateVertexBuffer(bufferSize, this->GetFVF());
    if (m_HWVertexBuffer == nullptr)
    {
      //failed
      m_HWVertexBufferSize = 0;
      return;
    }
  }
  
  void* recv = nullptr;
  if SUCCEEDED(m_HWVertexBuffer->Lock(0, bufferSize, reinterpret_cast<void**>(&recv), 0))
  {
    memcpy(recv, this->GetVertexData(), bufferSize);
    m_HWVertexBuffer->Unlock();
  }
  else {
    check(false);
  }
  m_HWVertexBufferSize = max(bufferSize, m_HWVertexBufferSize);
}

void VertexBufferI::Release()
{
  if (m_HWVertexBuffer != nullptr)
  {
    m_HWVertexBuffer->Release();
    m_HWVertexBuffer = nullptr;
  }
  m_HWVertexBufferSize = 0;
}