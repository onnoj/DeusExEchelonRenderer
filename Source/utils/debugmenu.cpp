#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "debugmenu.h"
#include "hacks/misc.h"
#include "uefacade.h"

#include <imgui.cpp>
#include <imgui_draw.cpp>
#include <imgui_tables.cpp>
#include <imgui_widgets.cpp>
#include <imgui_demo.cpp>
#include <backends/imgui_impl_win32.cpp>
#include <backends/imgui_impl_sdl2.cpp>
#include <backends/imgui_impl_dx11.cpp>

#include <map>

#pragma comment(lib, "d3d11.lib")

DebugMenu g_DebugMenu;

void DebugMenu::Init()
{
  if (!EE_HAS_IMGUI() || m_Initialized) return;
  // We have to use SDL+DX11 because the nvRemix wrapper is incompatible with imGui's multi viewport feature.

  // Setup SDL
  // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
  // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to the latest version of SDL is recommended!)
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
  {
    OutputDebugStringA("SDL Error: ");
    OutputDebugStringA(SDL_GetError());
    OutputDebugStringA("\n");
    return;
  }
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN);
  m_SDLWindow = SDL_CreateWindow("Dear ImGui SDL2+DirectX11 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1, 1, window_flags);
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(m_SDLWindow, &wmInfo);
  m_HWND = (HWND)wmInfo.info.win.window;

  // Initialize Direct3D
  if (!CreateDeviceD3D(m_HWND))
  {
    CleanupDeviceD3D();
    return;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGui::StyleColorsDark();

  ImGui_ImplWin32_EnableDpiAwareness();

  ImGui_ImplSDL2_InitForD3D(m_SDLWindow);
  ImGui_ImplDX11_Init(m_D3DDevice, m_D3DDeviceContext); 
  m_Initialized = true;
}

void DebugMenu::Shutdown()
{
  if (!EE_HAS_IMGUI() || !m_Initialized) return;
  m_Initialized = false;
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  CleanupRenderTarget();
  CleanupDeviceD3D();
  ImGui::DestroyContext();
}

void DebugMenu::Render()
{
  if (!EE_HAS_IMGUI() || !m_Initialized) return;
  // Poll and handle events (inputs, window resize, etc.)
  // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
  // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
  // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
  // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED && event.window.windowID == SDL_GetWindowID(m_SDLWindow))
    {
      // Release all outstanding references to the swap chain's buffers before resizing.
      CleanupRenderTarget();
      m_SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
      CreateRenderTarget();
    }
  }

  ImGui_ImplDX11_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
  //imguiDockId = ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_NoDockingInCentralNode);
  {
    Update();
    ImGui::Render();
    const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_D3DDeviceContext->OMSetRenderTargets(1, &m_MainRenderTargetView, nullptr);
    m_D3DDeviceContext->ClearRenderTargetView(m_MainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
  }
  ImGui::EndFrame();
  ImGui::UpdatePlatformWindows();
  ImGui::RenderPlatformWindowsDefault();
  m_SwapChain->Present(0, 0); // Present with vsync
}

bool DebugMenu::CreateDeviceD3D(HWND hWnd)
{
  if (!EE_HAS_IMGUI()) return false;

  // Setup swap chain
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT createDeviceFlags = 0;
  //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
  if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_SwapChain, &m_D3DDevice, &featureLevel, &m_D3DDeviceContext) != S_OK)
    return false;

  CreateRenderTarget();
  return true;
}

void DebugMenu::CleanupDeviceD3D()
{
  CleanupRenderTarget();
  if (m_SwapChain) { m_SwapChain->Release(); m_SwapChain = nullptr; }
  if (m_D3DDeviceContext) { m_D3DDeviceContext->Release(); m_D3DDeviceContext = nullptr; }
  if (m_D3DDevice) { m_D3DDevice->Release(); m_D3DDevice = nullptr; }
}

void DebugMenu::CreateRenderTarget()
{
  if (!EE_HAS_IMGUI()) return;

  ID3D11Texture2D* pBackBuffer;
  m_SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  m_D3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_MainRenderTargetView);
  pBackBuffer->Release();
}

void DebugMenu::CleanupRenderTarget()
{
  if (m_MainRenderTargetView) { m_MainRenderTargetView->Release(); m_MainRenderTargetView = nullptr; }
}

const char* DebugMenu::getTypeFormat(DebugMenuTypes pType)
{
  switch (pType)
  {
    case DebugMenuTypes::Integer: return "%d";
    case DebugMenuTypes::Float:return "%f";
    case DebugMenuTypes::Boolean:return "%d";
    case DebugMenuTypes::Vector3f: return "%f, %f, %f";
    case DebugMenuTypes::Vector4f:return "%f, %f, %f, %f";
    case DebugMenuTypes::FPlane:return "X:%f, Y:%f, Z:%f, W:%f";
    case DebugMenuTypes::UETexture:return "%d";
    case DebugMenuTypes::String: return "%s";
    case DebugMenuTypes::Lambda: return nullptr;
    case DebugMenuTypes::Undefined: [[fallthrough]];
    default:  assert(false); return "{}";
  }
}

void DebugMenu::getTypeVaListRead(std::function<void(std::va_list& outList)> pFunctor, DebugMenuTypes pType, void* pData)
{
  auto recombinator = [&](int dummy, ...)
  {
    std::va_list args;
    va_start(args, dummy);
    pFunctor(args);
    va_end(args);
  };

  switch (pType)
  {
    case DebugMenuTypes::Integer: recombinator(1, *reinterpret_cast<int*>(pData)); break;
    case DebugMenuTypes::Float: recombinator(1, *reinterpret_cast<float*>(pData)); break;
    case DebugMenuTypes::Vector3f:  recombinator(3, 
      reinterpret_cast<float*>(pData)[0],
      reinterpret_cast<float*>(pData)[1],
      reinterpret_cast<float*>(pData)[2]
    ); break;
    case DebugMenuTypes::Vector4f: recombinator(4, 
      reinterpret_cast<float*>(pData)[0],
      reinterpret_cast<float*>(pData)[1],
      reinterpret_cast<float*>(pData)[2],
      reinterpret_cast<float*>(pData)[3]
    ); break;
    case DebugMenuTypes::FPlane: recombinator(4,
      reinterpret_cast<FPlane*>(pData)->X,
      reinterpret_cast<FPlane*>(pData)->Y,
      reinterpret_cast<FPlane*>(pData)->Z,
      reinterpret_cast<FPlane*>(pData)->W
    ); break;
    case DebugMenuTypes::Boolean: recombinator(1, *reinterpret_cast<bool*>(pData)); break;
    case DebugMenuTypes::UETexture: recombinator(1, *reinterpret_cast<int*>(pData)); break;
    case DebugMenuTypes::String: {
      const char* source = reinterpret_cast<std::string*>(pData)->c_str();
      
      recombinator(1, source);
    } break;
    case DebugMenuTypes::Lambda: break;
    case DebugMenuTypes::Undefined: [[fallthrough]];
    default: assert(false);
  }
}

void DebugMenu::getTypeVaListWrite(std::function<void(std::va_list& outList)> pFunctor, DebugMenuTypes pType, void* pData)
{
  auto recombinator = [&](int dummy, ...)
  {
    std::va_list args;
    va_start(args, dummy);
    pFunctor(args);
    va_end(args);
  };

  switch (pType)
  {
  case DebugMenuTypes::Integer: recombinator(1, reinterpret_cast<int*>(pData)); break;
  case DebugMenuTypes::Float: recombinator(1, reinterpret_cast<float*>(pData)); break;
  case DebugMenuTypes::Vector3f:  recombinator(1, 
    &reinterpret_cast<float*>(pData)[0],
    &reinterpret_cast<float*>(pData)[1],
    &reinterpret_cast<float*>(pData)[2]
  ); break;
  case DebugMenuTypes::Vector4f: recombinator(1, 
    &reinterpret_cast<float*>(pData)[0],
    &reinterpret_cast<float*>(pData)[1],
    &reinterpret_cast<float*>(pData)[2],
    &reinterpret_cast<float*>(pData)[3]
  ); break;
  case DebugMenuTypes::FPlane: recombinator(4,
    &reinterpret_cast<FPlane*>(pData)->X,
    &reinterpret_cast<FPlane*>(pData)->Y,
    &reinterpret_cast<FPlane*>(pData)->Z,
    &reinterpret_cast<FPlane*>(pData)->W
  ); break;
  case DebugMenuTypes::Boolean: recombinator(1, reinterpret_cast<bool*>(pData)); break;
  case DebugMenuTypes::UETexture: recombinator(1, reinterpret_cast<int*>(pData)); break;
  case DebugMenuTypes::String: {
    std::string& target = *reinterpret_cast<std::string*>(pData);
    char b[1000]{ 0 };
    recombinator(1, &b[0]);
    target = b;
  } break;
  case DebugMenuTypes::Lambda: break;
  case DebugMenuTypes::Undefined: [[fallthrough]];
  default: assert(false);
  }
}

void DebugMenu::Update()
{
  if (!EE_HAS_IMGUI() || !m_Initialized) return;
  if (!g_options.hasDebugMenu)
  {
    return;
  }
  ImGui::Begin("Deus Ex FP Remix Renderer");
  ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Set window background to red

  //DPI scaling
  {
    const float dpiScale = ImGui_ImplWin32_GetDpiScaleForHwnd(ImGui::GetWindowViewport()->PlatformHandleRaw);
    ImGui::GetWindowViewport()->DpiScale = dpiScale;
    static auto defaultStyle = ImGui::GetStyle();
    ImGuiStyle& style = ImGui::GetStyle();
    style = defaultStyle;
    style.ScaleAllSizes(dpiScale);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //for (auto f : io.Fonts->Fonts)
    //{
    //  f->Scale = dpiScale;
    //}
    io.FontGlobalScale = dpiScale;
    //io.DisplayFramebufferScale.x = dpiScale;
    //io.DisplayFramebufferScale.y = dpiScale;
  }
  
#if 1
  for (auto& categoryPair : m_DebugItems)
  {
    auto& tupleOfDebugCategoryData = categoryPair.second;
    std::string categoryName = std::get<0>(tupleOfDebugCategoryData);
    bool& expanded = std::get<1>(tupleOfDebugCategoryData);
    auto& debugItems = *(std::get<2>(tupleOfDebugCategoryData));
    if (ImGui::CollapsingHeader(categoryName.c_str(), expanded ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None))
    {
      expanded = true;
      for (auto& itemPair : debugItems)
      {
        const auto& item = itemPair.second;
        std::string id = std::string("##") + item->identifierString;

        bool isEditing = (item->op == DebugItemValue::operation::writeToSrc);
        ImGui::Checkbox((id + "##checkbox").c_str(), &isEditing);
        ImGui::SameLine();
        ImGui::Text("%s: ", item->displayName.c_str());
        ImGui::SameLine();
        if (item->storageType == DebugMenuTypes::UETexture)
        {
          const UETextureType& textureDesc = *reinterpret_cast<UETextureType*>(item->storage);
          auto texture = FindTexture(*textureDesc);
          if (texture)
          {
            const ImVec2 posR = ImGui::GetCursorScreenPos();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            auto d = ((ImGui::GetColumnWidth() + ImGui::GetFrameHeight()) * 0.5f);
            auto hw = d * 1.0f;
            draw_list->AddImage((ImTextureID)texture->m_TextureSRV, posR + ImVec2(0, 0), posR + ImVec2(hw, hw));
            ImGui::SetCursorScreenPos(posR + ImVec2(0, hw + 10.0f));
            ImGui::ItemSize(ImRect(0, hw, 0, hw));
          }
          continue;
        }
        else if (item->storageType == DebugMenuTypes::Lambda)
        {
          if (ImGui::Button((id + "##lambdabutton").c_str(), ImVec2(20, 20)))
          {
            auto functor = *reinterpret_cast<std::function<void()>*>(item->storage);
            functor();
          }
          continue;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////


        if (!isEditing)
        {
          item->op = DebugItemValue::operation::readFromSrc;
          getTypeVaListRead([&](va_list& valist) {
            ImGui::TextV(getTypeFormat(item->storageType), valist);
            }, item->storageType, item->storage);
        }
        else
        {
          item->op = DebugItemValue::operation::writeToSrc;
          if (item->valueOptions.editor == DebugMenuValueOptions::editor::slider)
          {
            if (item->storageType == DebugMenuTypes::Float && item->valueOptions.minRangeF && item->valueOptions.maxRangeF)
            {
              float* f = reinterpret_cast<float*>(item->storage);
              ImGui::SliderFloat((id + "##slider").c_str(), f, *item->valueOptions.minRangeF, *item->valueOptions.maxRangeF);
            }
            else if (item->storageType == DebugMenuTypes::Integer && item->valueOptions.minRangeI32 && item->valueOptions.maxRangeI32)
            {
              int32_t* i32 = reinterpret_cast<int32_t*>(item->storage);
              ImGui::SliderInt((id + "##slider").c_str(), i32, *item->valueOptions.minRangeI32, *item->valueOptions.maxRangeI32);
            }
          }
          else //if (item->valueOptions.editor == DebugMenuValueOptions::editor::txt)
          {
            auto handler = [&](va_list& valist) {
              char b[300]{};
              vsprintf_s(&b[0], std::size(b), getTypeFormat(item->storageType), valist);
              if (ImGui::InputText((id + "##txtfield").c_str(), &b[0], std::size(b)))
              {
                getTypeVaListWrite([&](va_list& valist) {
                  int wrote = vsscanf_s(&b[0], getTypeFormat(item->storageType), valist);
                  item->writeBackValid = (wrote > 0);
                  }, item->storageType, item->storage);
              }
              if (!item->writeBackValid)
              {
                ImGui::SameLine();
                ImGui::TextColored(ImColor(0xFF0000FF), "Invalid Data!");
              }
            };
            getTypeVaListRead(handler, item->storageType, item->storage);
          }
        }
      }
    }
    else
    {
      expanded = false;
    }
  }
#endif
  ImGui::PopStyleColor();
  ImGui::End();
}

const DebugMenu::TextureDesc* DebugMenu::FindTexture(uint32_t pCacheID) const
{
  if (!EE_HAS_IMGUI() || !m_Initialized) return nullptr;
  for (auto& t : m_TextureData)
  {
    //Don't cache if we're still in the revolver cache...
    if (t.m_TextureID == pCacheID)
    {
      return &t;
    }
  }
  return nullptr;
}

void DebugMenu::VisitTexture(FTextureInfo* pUETextureInfo)
{
  if (!EE_HAS_IMGUI() || !m_Initialized) return;
  if (FindTexture(pUETextureInfo->CacheID) != nullptr)
  {
    return;
  }
  auto& textureManager = ::Misc::g_Facade->GetHLRenderer()->GetTextureManager();
  auto textures = textureManager.FindTextures(pUETextureInfo->CacheID);
  if (textures.empty())
  {
    return;
  }

  const auto& textureHandle = textures[0];

  // Describe the texture
  D3D11_TEXTURE2D_DESC texDesc;
  ZeroMemory(&texDesc, sizeof(texDesc));
  texDesc.Width = textureHandle->md.width;
  texDesc.Height = textureHandle->md.height;
  texDesc.MipLevels = 1;
  texDesc.ArraySize = 1;
  switch (textureHandle->format)
  {
    case D3DFMT_A8R8G8B8: texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break;
    case D3DFMT_DXT1: texDesc.Format = DXGI_FORMAT_BC1_UNORM; break;
    default: return;
  }
  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  texDesc.CPUAccessFlags = 0;
  texDesc.MiscFlags = 0;
  
  auto& textureData = m_TextureData[m_FreeTextureIndex];
  
  if (textureData.m_TextureResource != nullptr)
  {
    textureData.m_TextureSRV->Release();
    textureData.m_TextureResource->Release();
    textureData.m_TextureSRV = nullptr;
    textureData.m_TextureResource = nullptr;
  }
  m_FreeTextureIndex = (m_FreeTextureIndex + 1) % std::size(m_TextureData);
  HRESULT hr = m_D3DDevice->CreateTexture2D(&texDesc, nullptr, &textureData.m_TextureResource);
  if (SUCCEEDED(hr))
  {
    m_D3DDeviceContext->UpdateSubresource(textureData.m_TextureResource, 0, nullptr, textureHandle->textureDataPtr, textureHandle->textureDataPitch, 0);
  
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = texDesc.Format; // BC1 compressed format
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    HRESULT hr = m_D3DDevice->CreateShaderResourceView(textureData.m_TextureResource, &srvDesc, &textureData.m_TextureSRV);
    check(SUCCEEDED(hr));
    textureData.m_TextureID = pUETextureInfo->CacheID;
  }
  
}