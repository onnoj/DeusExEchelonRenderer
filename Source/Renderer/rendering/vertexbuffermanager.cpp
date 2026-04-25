#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "vertexbuffermanager.h"

VertexBufferI::~VertexBufferI()
{
  Release();
}

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