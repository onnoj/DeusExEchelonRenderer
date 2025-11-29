#pragma once

#include "vertexformats.h"

#include <memory>
#include <unordered_map>

class VertexBufferI
{
public:
  VertexBufferI() = default;
  VertexBufferI(const VertexBufferI&) = delete;
  VertexBufferI(VertexBufferI&&) = delete;
  VertexBufferI& operator=(const VertexBufferI&) = delete;
  VertexBufferI& operator=(VertexBufferI&&) = delete;
  virtual ~VertexBufferI() = default;

  IDirect3DVertexBuffer9* GetBuffer() { return m_HWVertexBuffer; }
  const IDirect3DVertexBuffer9* GetBuffer() const { return m_HWVertexBuffer; }
  bool HasCommit() const { return !m_Dirty; }
  void Release();
  void Commit(struct RenderContext& ctx);
  void FlagDirty() { m_Dirty = true; }

  virtual const void* GetVertexData() const = 0;
  virtual std::size_t GetVertexSize() const = 0;
  virtual std::size_t GetVertexCount() const = 0;
  virtual uint32_t GetFVF() const = 0;
  virtual void Reset() = 0;
private:
  IDirect3DVertexBuffer9* m_HWVertexBuffer = nullptr;
  uint32_t m_HWVertexBufferSize = 0;
  bool m_Dirty = true;
};

template <typename TVertexFormat>
class VertexBuffer : public VertexBufferI
{
private:
  std::vector<TVertexFormat> m_Data;
public:
  template <size_t TSize>
  void Assign(const TVertexFormat (&&pArgs)[TSize])
  {
    m_Data.resize(TSize);
    memcpy(m_Data.data(), &pArgs[0], TSize * sizeof(TVertexFormat));
    FlagDirty();
  }

  void PushVertex(const TVertexFormat &&pArgs)
  {
    m_Data.push_back(std::move(pArgs));
    FlagDirty();
  }

  template <size_t TSize>
  void PushFace(const TVertexFormat(&& pArgs)[TSize])
  {
    for (auto&& it : pArgs)
    {
      m_Data.push_back(it);
    }
    FlagDirty();
  }

  uint32_t CalculateHash()
  {
    uint32_t hash;
    MurmurHash3_x86_32(m_Data.data(), m_Data.size() * sizeof(m_Data[0]), 0, &hash);
    return hash;
  }

  virtual void Reset()
  {
    m_Data.clear();
  }

  virtual const void* GetVertexData() const { return static_cast<const void*>(&m_Data[0]); }
  virtual std::size_t GetVertexSize() const { return sizeof(TVertexFormat); }
  virtual std::size_t GetVertexCount() const { return m_Data.size(); }

  virtual uint32_t GetFVF() const;
};

class VertexBufferManager
{
  using BufferMap = std::unordered_map<uint64_t, std::unique_ptr<VertexBufferI*>>;
private:
  BufferMap m_Storage;
public:
  template <typename TVertexFormat>
  void Allocate()
  {
    auto ptr = std::make_unique< VertexBuffer<TVertexFormat> >();
    const uint64_t key = static_cast<uint64_t>(ptr.get());
    m_Storage.insert({key, std::move(ptr)});
  }
};