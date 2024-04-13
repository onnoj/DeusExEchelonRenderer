#pragma once

#include <d3d11.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <string>
#include <cstdarg>
#include <map>

#ifndef TOKENIZE
#define TOKENIZE2(s) #s
#define TOKENIZE(s) TOKENIZE2(s)
#endif
//#define DebugMenuUniqueID() (g_options.hasDebugMenu ? (()).c_str()) : nullptr)
#define DebugMenuUniqueID() (g_options.hasDebugMenu ? ([]()->uint64_t{                  \
  static std::string s = (std::string(__FILE__) + std::string(TOKENIZE(__LINE__)));     \
  static uint64_t key = (uint64_t(__LINE__)<<32) | uint64_t(std::hash<std::string>()(s));         \
  return key;                                                                           \
}()) : 0ull)

using UETextureType = std::optional<uint32_t>;

struct DebugMenuValueOptions
{
  enum class editor { txt, slider } editor = editor::txt;
  std::optional<float> minRangeF;
  std::optional<float> maxRangeF;
  std::optional<int32_t> minRangeI32;
  std::optional<int32_t> maxRangeI32;
};

class DebugMenu
{
public:
  struct TextureDesc;
public:
  enum class DebugMenuTypes
  {
    Undefined,
    Label,
    Boolean,
    Integer,
    Float,
    Vector3f,
    Vector4f,
    FPlane,
    UETexture,
    String,
    Lambda,
  };
public:
  void Init();
  void Shutdown();
  void Update();
  void Render();

  void VisitTexture(FTextureInfo* pUETextureInfo);
  
  template <typename T>
  void DebugVar(const char* pCategory, const char* pDisplayName, uint64_t pIdentifier, T& pmValue, DebugMenuValueOptions options = {});
protected:
  bool CreateDeviceD3D(HWND _hwnd);
  void CleanupDeviceD3D();
  void CreateRenderTarget();
  void CleanupRenderTarget();
  const TextureDesc* FindTexture(uint32_t pCacheID) const;

  template <typename T>
  DebugMenuTypes getTypeIndex();

  const char* getTypeFormat(DebugMenuTypes pType);
  void getTypeVaListRead(std::function<void(std::va_list& outList)> pFunctor, DebugMenuTypes pType, void* pData);
  void getTypeVaListWrite(std::function<void(std::va_list& outList)> pFunctor, DebugMenuTypes pType, void* pData);
private:
  using DebugItemKey = uint64_t;
  struct DebugItemValue {
    std::string category;
    std::string displayName;
    DebugItemKey identifier;
    std::string identifierString;
    DebugMenuTypes storageType = DebugMenuTypes::Undefined;
    void* storage = nullptr;
    uint32_t storageSize = 0;
    bool writeBackValid = true;
    DebugMenuValueOptions valueOptions{};
    enum class operation {readFromSrc, writeToSrc} op = operation::readFromSrc;
  };
private:
  ID3D11Device* m_D3DDevice = nullptr;
  ID3D11DeviceContext* m_D3DDeviceContext = nullptr;
  IDXGISwapChain* m_SwapChain = nullptr;
  ID3D11RenderTargetView* m_MainRenderTargetView = nullptr;
  SDL_Window* m_SDLWindow = nullptr;
  WNDPROC m_OriginalWNDPROC{};
  HWND m_HWND{};
  bool m_Initialized = false;
  uint32_t m_TextureIDs[16]{};

  struct TextureDesc
  {
    uint32_t m_TextureID = 0;
    ID3D11ShaderResourceView* m_TextureSRV = nullptr;
    ID3D11Texture2D* m_TextureResource = nullptr;
  } m_TextureData[16];
  uint8_t m_FreeTextureIndex = 0;

  using DebugMap = std::map<DebugItemKey, std::shared_ptr<DebugItemValue>>;
  using CategoryDebugMap = std::map<DebugItemKey, std::tuple<std::string/*category*/, bool/*expanded*/, std::shared_ptr<DebugMap>>>;
  CategoryDebugMap m_DebugItems;
};
extern DebugMenu g_DebugMenu;

template <typename T>
DebugMenu::DebugMenuTypes DebugMenu::getTypeIndex()
{
  if constexpr (std::is_same<T, float>::value)
  {
    return DebugMenuTypes::Float;
  }
  if constexpr (std::is_same<T, uint32_t>::value)
  {
    return DebugMenuTypes::Integer;
  }
  if constexpr (std::is_same<T, int32_t>::value)
  {
    return DebugMenuTypes::Integer;
  }
  if constexpr (std::is_same<T, uint64_t>::value)
  {
    return DebugMenuTypes::Integer;
  }
  if constexpr (std::is_same<T, int64_t>::value)
  {
    return DebugMenuTypes::Integer;
  }
  if constexpr (std::is_same<T, unsigned long>::value)
  {
    return DebugMenuTypes::Integer;
  }
  if constexpr (std::is_same<T, FPlane>::value)
  {
    return DebugMenuTypes::FPlane;
  }
  if constexpr (std::is_same<T, FVector>::value)
  {
    return DebugMenuTypes::Vector3f;
  }
  if constexpr (std::is_same<T, D3DVECTOR>::value)
  {
    return DebugMenuTypes::Vector3f;
  }
  if constexpr (std::is_same<T, bool>::value)
  {
    return DebugMenuTypes::Boolean;
  }
  if constexpr (std::is_same<T, UETextureType>::value)
  {
    return DebugMenuTypes::UETexture;
  }
  if constexpr (std::is_same<T, std::string>::value)
  {
    return DebugMenuTypes::String;
  }
  if constexpr (std::is_same<T, std::function<void()>>::value)
  {
    return DebugMenuTypes::Lambda;
  }
  if constexpr (std::is_convertible<T, uint32_t>::value)
  {
    return DebugMenuTypes::Integer;
  }
  return DebugMenuTypes::Undefined;
}

template <typename T>
void DebugMenu::DebugVar(const char* pCategory, const char* pDisplayName, uint64_t pIdentifier, T& pmValue, DebugMenuValueOptions options)
{
  if (!g_options.hasDebugMenu)
  {
    return;
  }

  DebugItemValue* value = nullptr;
  uint32_t categoryKey = 0;
  MurmurHash3_x86_32(pCategory, strlen(pCategory), 0, &categoryKey);
  auto categoryIt = m_DebugItems.find(categoryKey);
  if (categoryIt == m_DebugItems.end())
  {
    categoryIt = m_DebugItems.emplace(categoryKey, std::make_tuple(pCategory, false, std::make_shared<DebugMap>())).first;
  }

  const uint64_t key = pIdentifier;
  
  DebugMap& itemMap = *std::get<2>(categoryIt->second);//*((*categoryIt).second.second.get());
  auto itemIt = itemMap.find(key);
  if (itemIt == itemMap.end())
  {
    itemIt = itemMap.emplace(key, std::make_shared<DebugItemValue>()).first;
    value = itemIt->second.get();
    value->category = pCategory;
    value->displayName = pDisplayName;
    value->identifier = pIdentifier;
    using storageType = T;
    value->storageType = getTypeIndex<storageType>();
    value->storage = new(storageType);
    value->storageSize = sizeof(storageType);
    value->valueOptions = options;
    value->op = DebugItemValue::operation::readFromSrc;
    char b[64]{};
    sprintf(&b[0], "%lld", value->identifier);
    value->identifierString = &b[0];
  }
  else
  {
    value = itemIt->second.get();
  }

  if (value->op == DebugItemValue::operation::readFromSrc)
  {
    if constexpr (std::is_pod<T>())
    {
      memcpy(value->storage, &pmValue, value->storageSize);
    }
    else
    {
      *reinterpret_cast<T*>(value->storage) = pmValue;
    }
  }
  else if (value->op == DebugItemValue::operation::writeToSrc)
  {
    if constexpr (std::is_pod<T>())
    {
      memcpy(&pmValue, value->storage, value->storageSize);
    }
    else
    {
      pmValue = *reinterpret_cast<T*>(value->storage);
    }
  }
}
