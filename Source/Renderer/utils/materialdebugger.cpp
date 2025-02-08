#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include <filesystem>

#include "uefacade.h"
#include "hacks/misc.h"
#include "utils/debugmenu.h"
#include "utils/utils.h"
#include "materialdebugger.h"
#include "rendering/llrenderer.h"
#include "rendering/hlrenderer.h"

#include <d3d9.h>

#include <nlohmann/json.hpp>

MaterialDebugger g_MaterialDebugger;

namespace
{
  constexpr std::pair<EBspNodeFlags, const char*> knownNodeFlags[] =
  {
    { NF_NotCsg, "NF_NotCsg"},
    { NF_ShootThrough, "NF_ShootThrough"},
    { NF_NotVisBlocking, "NF_NotVisBlocking"},
    { NF_PolyOccluded, "NF_PolyOccluded"},
    { NF_BoxOccluded, "NF_BoxOccluded"},
    { NF_BrightCorners, "NF_BrightCorners"},
    { NF_IsNew, "NF_IsNew"},
    { NF_IsFront, "NF_IsFront"},
    { NF_IsBack, "NF_IsBack"},
  };

  constexpr std::pair<UnrealPolyFlags, const char*> knownPolyFlags[] =
  {
    {PF_Invisible, "PF_Invisible"},
    {PF_Masked, "PF_Masked"},
    {PF_Translucent, "PF_Translucent"},
    {PF_NotSolid, "PF_NotSolid"},
    {PF_Environment, "PF_Environment"},
    {PF_ForceViewZone, "PF_ForceViewZone"},
    {PF_Semisolid, "PF_Semisolid"},
    {PF_Modulated, "PF_Modulated"},
    {PF_FakeBackdrop, "PF_FakeBackdrop"},
    {PF_TwoSided, "PF_TwoSided"},
    {PF_AutoUPan, "PF_AutoUPan"},
    {PF_AutoVPan, "PF_AutoVPan"},
    {PF_NoSmooth, "PF_NoSmooth"},
    {PF_BigWavy, "PF_BigWavy"},
    {PF_SpecialPoly, "PF_SpecialPoly"},
    {PF_SmallWavy, "PF_SmallWavy"},
    {PF_Flat, "PF_Flat"},
    {PF_LowShadowDetail, "PF_LowShadowDetail"},
    {PF_NoMerge, "PF_NoMerge"},
    {PF_CloudWavy, "PF_CloudWavy"},
    {PF_DirtyShadows, "PF_DirtyShadows"},
    {PF_BrightCorners, "PF_BrightCorners"},
    {PF_SpecialLit, "PF_SpecialLit"},
    {PF_Gouraud, "PF_Gouraud"},
    {PF_NoBoundRejection, "PF_NoBoundRejection"},
    {PF_Unlit, "PF_Unlit"},
    {PF_HighShadowDetail, "PF_HighShadowDetail"},
    {PF_Portal, "PF_Portal"},
    {PF_Mirrored, "PF_Mirrored"},
    {PF_Memorized, "PF_Memorized"},
    {PF_Selected, "PF_Selected"},
    {PF_Highlighted, "PF_Highlighted"},
    {PF_FlatShaded, "PF_FlatShaded"},
    {PF_EdProcessed, "PF_EdProcessed"},
    {PF_EdCut, "PF_EdCut"},
    {PF_RenderFog, "PF_RenderFog"},
    {PF_Occlude, "PF_Occlude"},
    {PF_RenderHint, "PF_RenderHint"},
    {PF_NoOcclude, "PF_NoOcclude"},
    {PF_NoEdit, "PF_NoEdit"},
    {PF_NoImport, "PF_NoImport"},
    {PF_AddLast, "PF_AddLast"},
    {PF_NoAddToBSP, "PF_NoAddToBSP"},
    {PF_NoShadows, "PF_NoShadows"},
    {PF_Transient, "PF_Transient"},
  };
}

void MaterialDebugger::nfDecoderUtil()
{
  uint32_t nodeFlags = 0;
  g_DebugMenu.DebugVar("Modding - Utils", "NodeFlag ID", DebugMenuUniqueID(), nodeFlags);
  if (nodeFlags != 0)
  {
    std::string nodeFlagsTxt = "(none)";
    for (auto& nf : knownNodeFlags)
    {
      if ((nodeFlags & nf.first) != 0)
      {
        if (!nodeFlagsTxt.empty())
          nodeFlagsTxt += "\r\n";
        nodeFlagsTxt += nf.second;
      }
    }

    g_DebugMenu.DebugVar("Modding - Utils", "Node Flags", DebugMenuUniqueID(), nodeFlagsTxt);
  }
}

void MaterialDebugger::pfDecoderUtil()
{
  uint32_t polyFlags = 0;
  g_DebugMenu.DebugVar("Modding - Utils", "PolyFlag ID", DebugMenuUniqueID(), polyFlags);
  if (polyFlags != 0)
  {
    std::string polyFlagsTxt = "(none)";
    for (auto& nf : knownPolyFlags)
    {
      if ((polyFlags & nf.first) != 0)
      {
        if (!polyFlagsTxt.empty())
          polyFlagsTxt += "\r\n";
        polyFlagsTxt += nf.second;
      }
    }

    g_DebugMenu.DebugVar("Modding - Utils", "Poly Flags", DebugMenuUniqueID(), polyFlagsTxt);
  }
}


void MaterialDebugger::Update(FSceneNode* Frame)
{
  auto ctx = g_ContextManager.GetContext();
  static bool debugMenuEnabled = false;
  nfDecoderUtil();
  pfDecoderUtil();

  //export util
  if (ctx && !ctx->frameIsRasterized && !ctx->frameIsSkybox && !ctx->renderingUI)
  {
    g_DebugMenu.DebugVar("Modding - Export", "Export [RTX Hash <> Name] mapping of\ncurrently loaded textures", DebugMenuUniqueID(), std::function<void()>([&]() {
      exportHashMappings(false);
      }));
    g_DebugMenu.DebugVar("Modding - Export", "Export [RTX Hash <> Name] mapping\nof all textures", DebugMenuUniqueID(), std::function<void()>([&]() {
      exportHashMappings(true);
      }));

    bool renderAllTextures = false;
    g_DebugMenu.DebugVar("Modding - Export", "Emit all textures each frame", DebugMenuUniqueID(), renderAllTextures);
    if (renderAllTextures)
    {
      this->renderAllTextures();
    }
  }

  //Inspector
  g_DebugMenu.DebugVar("Modding - Inspector", "Enabled", DebugMenuUniqueID(), debugMenuEnabled);
  if(!debugMenuEnabled)
  {
    return;
  }
  ///
  FVector dir = -(Frame->Coords.XAxis ^ Frame->Coords.YAxis);
  FVector begin = Frame->Coords.Origin;
  FVector end = Frame->Coords.Origin + dir * 1000000.0f;
  FCheckResult res(0.0f);
  auto levelModel = Frame->Level->Model;
  UETextureType textureID = 0;
  std::string textureName;
  UnrealPolyFlags surfFlags = 0;
  UnrealPolyFlags textureFlags = 0;
  FBspNode* node = nullptr;
  AActor* actor = nullptr;
  uint32_t iNode = 0;
  uint32_t iActor = 0;
  uint32_t iMesh = 0;
  int32_t iSurf = 0;
  std::string actorName;
  if (Frame->Level->SingleLineCheck(res, nullptr, end, begin, TRACE_AllEverything) == 0)
  {
    for (auto raycast = &res; raycast != nullptr; raycast = raycast->GetNext())
    {
      if (raycast->Actor == nullptr || raycast->Actor->IsA(ALevelInfo::StaticClass()))
      {
        if (res.Item != INDEX_NONE && res.Item != 0xccccccccul)
        {
          iNode = (uint32_t)res.Item;
          if (iNode >= 0 && iNode < levelModel->Nodes.Num())
          {
            node = &levelModel->Nodes(iNode);
            iSurf = node->iSurf;
            if (iSurf >= 0 && iSurf < levelModel->Surfs.Num())
            {
              const auto& surf = levelModel->Surfs(iSurf);
              const auto texture = surf.Texture;
              if (texture)
              {
                FTextureInfo info{};
                texture->Lock(info, 0, 0, GRenderDevice);
                textureID = info.CacheID;
                textureName = Utils::ConvertWcToUtf8(info.Texture->GetFullName());
                surfFlags = surf.PolyFlags;
                textureFlags = texture->PolyFlags;
                g_DebugMenu.VisitTexture(&info);
                texture->Unlock(info);
                break;
              }
              else
              {
                int x = 1;
              }
            }
          }
        }
      }
      else if (raycast->Actor)
      {
        actor = raycast->Actor;
        iActor = raycast->Actor->GetIndex();
        actorName = Utils::ConvertWcToUtf8(raycast->Actor->GetFullName());
        UTexture* texture = nullptr;
        if (raycast->Actor->Mesh != nullptr)
        {
          texture = raycast->Actor->Mesh->GetTexture(0, raycast->Actor);
          iMesh = raycast->Actor->Mesh->GetIndex();
        }

        if (texture == nullptr)
        {
          texture = raycast->Actor->Texture;
        }

        if (texture)
        {
          FTextureInfo info{};
          texture->Lock(info, 0, 0, GRenderDevice);
          textureID = info.CacheID;
          surfFlags = 0;
          textureFlags = texture->PolyFlags;
          g_DebugMenu.VisitTexture(&info);
          texture->Unlock(info);
        }
        else
        {
          continue;
        }

        if (raycast->Actor->Mesh != nullptr && raycast->Actor->Mesh->IsA(ULodMesh::StaticClass()))
        {
          auto lodMesh = ((ULodMesh*)raycast->Actor->Mesh);
          for (int i = 0; i < lodMesh->Materials.Num(); i++)
          {
            surfFlags |= lodMesh->Materials(i).PolyFlags;
          }
        }
        break;
      }
    }
  }

  //Check our texture cache, see if we have a nv remix hash ID.
  std::string remixTextureHashes;
  if (textureID)
  {
    auto& textureManager = Misc::g_Facade->GetHLRenderer()->GetTextureManager();
    auto textures = textureManager.FindTextures(*textureID);
    for (const auto& t : textures)
    {
      char b[32]{ 0 };
      ::sprintf(&b[0], "%016llx", t->remixHash);
      if (remixTextureHashes.find(&b[0]) == std::string::npos)
      {
        if (!remixTextureHashes.empty())
        {
          remixTextureHashes += ", ";
        }
        remixTextureHashes += &b[0];
      }
    }
  }


  //Process the node and primitive flags
  std::string nodeFlagsTxt = "(none)";
  if (node != nullptr && node->NodeFlags != 0)
  {
    for (auto& nf : knownNodeFlags)
    {
      if ((node->NodeFlags & nf.first) != 0)
      {
        if (!nodeFlagsTxt.empty())
          nodeFlagsTxt += "\r\n";
        nodeFlagsTxt += nf.second;
      }
    }
  }

  std::string surfaceFlagTxt = "";
  std::string textureFlagTxt = "";


  bool hasSurfaceFlags = false;
  bool hasTextureFlags = false;

  for (auto& pf : knownPolyFlags)
  {
    if ((surfFlags & pf.first) == pf.first)
    {
      if (!surfaceFlagTxt.empty())
        surfaceFlagTxt += "\r\n";
      surfaceFlagTxt += pf.second;
      hasSurfaceFlags = true;
    }

    if ((textureFlags & pf.first) == pf.first)
    {
      if (!textureFlagTxt.empty())
        textureFlagTxt += "\r\n";
      textureFlagTxt += pf.second;
      hasTextureFlags = true;
    }
  }
  if (!hasSurfaceFlags)  surfaceFlagTxt = "(none)";
  if (!hasTextureFlags)  textureFlagTxt = "(none)";

  std::string actorClassPath;
  if (actor)
  {
    for (auto cls = actor->GetClass(); cls != nullptr; cls = cls->GetSuperClass())
    {
      actorClassPath += Utils::ConvertWcToUtf8(cls->GetNameCPP());
      if (cls->GetSuperClass() != nullptr)
      {
        actorClassPath += " <- ";
      }
    }
  }

  g_DebugMenu.DebugVar("Modding - Inspector", "iNode", DebugMenuUniqueID(), iNode);
  g_DebugMenu.DebugVar("Modding - Inspector", "iSurf", DebugMenuUniqueID(), iSurf);
  g_DebugMenu.DebugVar("Modding - Inspector", "Remix Hashes", DebugMenuUniqueID(), remixTextureHashes);
  g_DebugMenu.DebugVar("Modding - Inspector", "Actor Index", DebugMenuUniqueID(), iActor);
  g_DebugMenu.DebugVar("Modding - Inspector", "Actor Class", DebugMenuUniqueID(), actorClassPath);
  g_DebugMenu.DebugVar("Modding - Inspector", "Mesh Index", DebugMenuUniqueID(), iMesh);
  g_DebugMenu.DebugVar("Modding - Inspector", "Actor Name", DebugMenuUniqueID(), actorName);
  g_DebugMenu.DebugVar("Modding - Inspector", "Viewed Texture Name", DebugMenuUniqueID(), textureName);
  g_DebugMenu.DebugVar("Modding - Inspector", "Viewed Texture", DebugMenuUniqueID(), textureID);
  g_DebugMenu.DebugVar("Modding - Inspector", "Node Flags", DebugMenuUniqueID(), nodeFlagsTxt);
  g_DebugMenu.DebugVar("Modding - Inspector", "Surface Flags", DebugMenuUniqueID(), surfaceFlagTxt);
  g_DebugMenu.DebugVar("Modding - Inspector", "Texture Flags", DebugMenuUniqueID(), textureFlagTxt);
}

void MaterialDebugger::renderAllTextures()
{
  auto facade = dynamic_cast<UD3D9FPRenderDevice*>(GRenderDevice);
  auto llRenderer = facade->GetLLRenderer();
  auto hlRenderer = facade->GetHLRenderer();
  auto& textureManager = hlRenderer->GetTextureManager();



#if 1
  auto ctx = g_ContextManager.GetContext();
  Utils::ScopedCall scopedRenderStatePushPop{ [&](){ llRenderer->PushDeviceState();}, [&]() {llRenderer->PopDeviceState(); } };
  
  hlRenderer->SetViewState(ctx->frameSceneNode, HighlevelRenderer::ViewType::game);
  hlRenderer->SetProjectionState(ctx->frameSceneNode, HighlevelRenderer::ProjectionType::perspective);
  llRenderer->SetRenderState(D3DRS_ZENABLE, 1);
  llRenderer->SetRenderState(D3DRS_ZWRITEENABLE, 1);
  llRenderer->SetRenderState(D3DRS_DEPTHBIAS, 1.0f);
  llRenderer->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

  //dump currently loaded textures
  uint32_t count = 0;
  int row = 0;
  int col = 0;

  for (FObjectIterator it = FObjectIterator(UTexture::StaticClass()); *it != nullptr; it.operator++())
  {
    if (col++ >= 70)
    {
      col = 0;
      row++;
    }

    auto texture = static_cast<UTexture*>(*it);
    if (!texture->IsA(UTexture::StaticClass()))
    {
      continue;
    }
    UObject* texturePackage = texture->GetOuter();

    count++;
    FTextureInfo textureInfo{};
    texture->Lock(textureInfo, 0, 0, GRenderDevice);
    auto md = textureManager.ProcessTexture(texture->PolyFlags, &textureInfo);

    //Do a dummy render call so that Remix picks up the texture
    {
      assert(ctx != nullptr);
      auto fwd = (ctx->frameSceneNode->Coords.XAxis ^ ctx->frameSceneNode->Coords.YAxis);
      auto widgetPos = ctx->frameSceneNode->Coords.Origin - (fwd * 800.0f);
      widgetPos -= ctx->frameSceneNode->Coords.XAxis * 10.0f * 35.0f;
      widgetPos += ctx->frameSceneNode->Coords.XAxis * 10.0f * float(col);
      widgetPos -= ctx->frameSceneNode->Coords.YAxis * 10.0f * 35.0f;
      widgetPos += ctx->frameSceneNode->Coords.YAxis * 10.0f * float(row);
#if defined(CONVERT_TO_LEFTHANDED_COORDINATES) && CONVERT_TO_LEFTHANDED_COORDINATES==1
      widgetPos.X *= -1.0f;
#endif
      hlRenderer->Draw3DCube(ctx->frameSceneNode, widgetPos, PF_Modulated, md, 10.0f);
    }
    texture->Unlock(textureInfo);
  }
  GLog->Logf(L"[EchelonRenderer]\t Rendered %d textures.", count);
#endif
}

void MaterialDebugger::exportHashMappings(bool pLoadAllTextures)
{
  auto facade = dynamic_cast<UD3D9FPRenderDevice*>(GRenderDevice);
  auto llRenderer = facade->GetLLRenderer();
  auto hlRenderer = facade->GetHLRenderer();
  auto& textureManager = hlRenderer->GetTextureManager();

  if (pLoadAllTextures)
  {
    auto paths = GSys->Paths;
    for (int i = 0; i < paths.Num(); i++)
    {
      auto searchPath = paths(i);
      if (auto extPos = searchPath.InStr(L"*.utx"); extPos >= 0)
      {
        FString packagePath = searchPath.Left(extPos);
        TArray<FString> textureFiles = GFileManager->FindFiles(*searchPath, 1, 0);
        for (int i = 0; i < textureFiles.Num(); i++)
        {
          auto textureFilePath = packagePath + textureFiles(i);
          GLog->Logf(L"[EchelonRenderer]\t Texture package: %s", *textureFilePath);
          if (GFileManager->FileSize(*textureFilePath) > 0)
          {
            UObject* package = UObject::LoadPackage(NULL, *textureFilePath, LOAD_None);
            if (package == nullptr || !package->IsA(UPackage::StaticClass()))
            {
GLog->Logf(L"[EchelonRenderer]\t Was not able to load texture package %s", *textureFilePath);
continue;
            }
            //UObject::ResetLoaders(package, 0, 1);
          }
        }
      }
    }
  }

  std::filesystem::path dllLocation = []() {
    wchar_t b[1024]{ 0 };
    GetModuleFileNameW(GetModuleHandle(NULL), &b[0], std::size(b));
    auto dllPath = std::filesystem::path(b);
    return dllPath.parent_path();
  }();

  auto jsonLocation = dllLocation / "textureDump.json";

  using json = nlohmann::json;
  auto rootObject = json::object();

  //dump currently loaded textures
  auto textureArray = json::array();
  for (auto it = FObjectIterator(UTexture::StaticClass()); it != FALSE; it.operator++())
  {
    auto texture = static_cast<UTexture*>(*it);
    if (!texture->IsA(UTexture::StaticClass()))
    {
      continue;
    }
    UObject* texturePackage = texture->GetOuter();

    FTextureInfo textureInfo{};
    texture->Lock(textureInfo, 0, 0, GRenderDevice);
    auto md = textureManager.ProcessTexture(texture->PolyFlags, &textureInfo);

    //Do a dummy render call so that Remix picks up the texture
    g_ContextManager.PushFrameContext();
    {
      auto ctx = g_ContextManager.GetContext();

      textureManager.BindTexture(texture->PolyFlags, md);
      hlRenderer->SetWorldTransformStateToIdentity();
      static LowlevelRenderer::VertexPos3Tex0to4 vbuffer;
      llRenderer->RenderTriangleList(&vbuffer, 1, 1, 0, 0);
    }
    g_ContextManager.PopFrameContext();

    if (textureInfo.Texture != nullptr)
    {
      auto textureObject = json::object();
      textureObject["package"] = Utils::ConvertWcToUtf8(texturePackage->GetPathName()).c_str();
      textureObject["path"] = Utils::ConvertWcToUtf8(textureInfo.Texture->GetPathName()).c_str();
      textureObject["name"] = Utils::ConvertWcToUtf8(textureInfo.Texture->GetName()).c_str();
      textureObject["cacheid"] = textureInfo.CacheID;
      textureObject["index"] = textureInfo.Texture->GetIndex();
      textureObject["remixhash"] = md->remixHash;
      textureObject["flags"] = json::array();
      textureObject["realtime"] = (textureInfo.bRealtime != 0);
      textureObject["usize"] = textureInfo.USize;
      textureObject["vsize"] = textureInfo.VSize;
      for (const auto& flag : knownPolyFlags)
      {
        if ((textureInfo.Texture->PolyFlags & flag.first) != 0)
        {
          textureObject["flags"].push_back(flag.second);
        }
      }

      char remixHashHex[64]{ 0 };
      ::sprintf(&remixHashHex[0], "%016llx", md->remixHash);
      textureObject["remixhash-hex"] = remixHashHex;
      textureArray.push_back(textureObject);
    }
    texture->Unlock(textureInfo);
  }
  rootObject["textures"] = textureArray;

  std::ofstream jsonDump(jsonLocation, std::ios_base::out | std::ios_base::trunc);
  if (jsonDump.is_open())
  {
    auto dumped = rootObject.dump(4);
    jsonDump.write(dumped.data(), dumped.length());
  }
  jsonDump.close();
}


////////////////////////////////////////////////////////////////////////////////////

void RenderStateDebugger::Process(LowlevelRenderer* pLLRenderer, uint32_t pDebugID)
{
  static uint32_t expectedDebugId = 0;
  g_DebugMenu.DebugVar("Rendering Debug", "\n\nRS Debug ID", DebugMenuUniqueID(), expectedDebugId);

  if (pDebugID != expectedDebugId || expectedDebugId == 0)
  {
    return;
  }

  using RenderStateInfo = std::tuple<uint64_t, D3DRENDERSTATETYPE, const char*>;
  
  constexpr auto getRenderStateDebugID = [](const RenderStateInfo& rsi) { return std::get<0>(rsi); };
  constexpr auto getRenderStateKey = [](const RenderStateInfo& rsi) { return std::get<1>(rsi); };
  constexpr auto getRenderStateName = [](const RenderStateInfo& rsi) { return std::get<2>(rsi); };
  static const RenderStateInfo renderstates[] = {
    {DebugMenuUniqueID(), D3DRS_ZENABLE, "D3DRS_ZENABLE"},
    {DebugMenuUniqueID(), D3DRS_FILLMODE, "D3DRS_FILLMODE"},
    {DebugMenuUniqueID(), D3DRS_SHADEMODE, "D3DRS_SHADEMODE"},
    {DebugMenuUniqueID(), D3DRS_ZWRITEENABLE, "D3DRS_ZWRITEENABLE"},
    {DebugMenuUniqueID(), D3DRS_ALPHATESTENABLE, "D3DRS_ALPHATESTENABLE"},
    {DebugMenuUniqueID(), D3DRS_LASTPIXEL, "D3DRS_LASTPIXEL"},
    {DebugMenuUniqueID(), D3DRS_SRCBLEND, "D3DRS_SRCBLEND"},
    {DebugMenuUniqueID(), D3DRS_DESTBLEND, "D3DRS_DESTBLEND"},
    {DebugMenuUniqueID(), D3DRS_BLENDOP, "D3DRS_BLENDOP"},
    {DebugMenuUniqueID(), D3DRS_BLENDFACTOR, "D3DRS_BLENDFACTOR"},
    {DebugMenuUniqueID(), D3DRS_ALPHABLENDENABLE, "D3DRS_ALPHABLENDENABLE"},
    {DebugMenuUniqueID(), D3DRS_SEPARATEALPHABLENDENABLE, "D3DRS_SEPARATEALPHABLENDENABLE"},
    {DebugMenuUniqueID(), D3DRS_SRCBLENDALPHA, "D3DRS_SRCBLENDALPHA"},
    {DebugMenuUniqueID(), D3DRS_DESTBLENDALPHA, "D3DRS_DESTBLENDALPHA"},
    {DebugMenuUniqueID(), D3DRS_BLENDOPALPHA, "D3DRS_BLENDOPALPHA"},
    {DebugMenuUniqueID(), D3DRS_VERTEXBLEND, "D3DRS_VERTEXBLEND"},
    {DebugMenuUniqueID(), D3DRS_INDEXEDVERTEXBLENDENABLE, "D3DRS_INDEXEDVERTEXBLENDENABLE"},
    {DebugMenuUniqueID(), D3DRS_CULLMODE, "D3DRS_CULLMODE"},
    {DebugMenuUniqueID(), D3DRS_ZFUNC, "D3DRS_ZFUNC"},
    {DebugMenuUniqueID(), D3DRS_ALPHAREF, "D3DRS_ALPHAREF"},
    {DebugMenuUniqueID(), D3DRS_ALPHAFUNC, "D3DRS_ALPHAFUNC"},
    {DebugMenuUniqueID(), D3DRS_DITHERENABLE, "D3DRS_DITHERENABLE"},
    {DebugMenuUniqueID(), D3DRS_FOGENABLE, "D3DRS_FOGENABLE"},
    {DebugMenuUniqueID(), D3DRS_SPECULARENABLE, "D3DRS_SPECULARENABLE"},
    {DebugMenuUniqueID(), D3DRS_FOGCOLOR, "D3DRS_FOGCOLOR"},
    {DebugMenuUniqueID(), D3DRS_FOGTABLEMODE, "D3DRS_FOGTABLEMODE"},
    {DebugMenuUniqueID(), D3DRS_FOGSTART, "D3DRS_FOGSTART"},
    {DebugMenuUniqueID(), D3DRS_FOGEND, "D3DRS_FOGEND"},
    {DebugMenuUniqueID(), D3DRS_FOGDENSITY, "D3DRS_FOGDENSITY"},
    {DebugMenuUniqueID(), D3DRS_RANGEFOGENABLE, "D3DRS_RANGEFOGENABLE"},
    {DebugMenuUniqueID(), D3DRS_STENCILENABLE, "D3DRS_STENCILENABLE"},
    {DebugMenuUniqueID(), D3DRS_STENCILFAIL, "D3DRS_STENCILFAIL"},
    {DebugMenuUniqueID(), D3DRS_STENCILZFAIL, "D3DRS_STENCILZFAIL"},
    {DebugMenuUniqueID(), D3DRS_STENCILPASS, "D3DRS_STENCILPASS"},
    {DebugMenuUniqueID(), D3DRS_STENCILFUNC, "D3DRS_STENCILFUNC"},
    {DebugMenuUniqueID(), D3DRS_STENCILREF, "D3DRS_STENCILREF"},
    {DebugMenuUniqueID(), D3DRS_STENCILMASK, "D3DRS_STENCILMASK"},
    {DebugMenuUniqueID(), D3DRS_STENCILWRITEMASK, "D3DRS_STENCILWRITEMASK"},
    {DebugMenuUniqueID(), D3DRS_TEXTUREFACTOR, "D3DRS_TEXTUREFACTOR"},
    {DebugMenuUniqueID(), D3DRS_WRAP0, "D3DRS_WRAP0"},
    {DebugMenuUniqueID(), D3DRS_WRAP1, "D3DRS_WRAP1"},
    {DebugMenuUniqueID(), D3DRS_WRAP2, "D3DRS_WRAP2"},
    {DebugMenuUniqueID(), D3DRS_WRAP3, "D3DRS_WRAP3"},
    {DebugMenuUniqueID(), D3DRS_WRAP4, "D3DRS_WRAP4"},
    {DebugMenuUniqueID(), D3DRS_WRAP5, "D3DRS_WRAP5"},
    {DebugMenuUniqueID(), D3DRS_WRAP6, "D3DRS_WRAP6"},
    {DebugMenuUniqueID(), D3DRS_WRAP7, "D3DRS_WRAP7"},
    {DebugMenuUniqueID(), D3DRS_CLIPPING, "D3DRS_CLIPPING"},
    {DebugMenuUniqueID(), D3DRS_LIGHTING, "D3DRS_LIGHTING"},
    {DebugMenuUniqueID(), D3DRS_AMBIENT, "D3DRS_AMBIENT"},
    {DebugMenuUniqueID(), D3DRS_FOGVERTEXMODE, "D3DRS_FOGVERTEXMODE"},
    {DebugMenuUniqueID(), D3DRS_COLORVERTEX, "D3DRS_COLORVERTEX"},
    {DebugMenuUniqueID(), D3DRS_LOCALVIEWER, "D3DRS_LOCALVIEWER"},
    {DebugMenuUniqueID(), D3DRS_NORMALIZENORMALS, "D3DRS_NORMALIZENORMALS"},
    {DebugMenuUniqueID(), D3DRS_DIFFUSEMATERIALSOURCE, "D3DRS_DIFFUSEMATERIALSOURCE"},
    {DebugMenuUniqueID(), D3DRS_SPECULARMATERIALSOURCE, "D3DRS_SPECULARMATERIALSOURCE"},
    {DebugMenuUniqueID(), D3DRS_AMBIENTMATERIALSOURCE, "D3DRS_AMBIENTMATERIALSOURCE"},
    {DebugMenuUniqueID(), D3DRS_EMISSIVEMATERIALSOURCE, "D3DRS_EMISSIVEMATERIALSOURCE"},
    {DebugMenuUniqueID(), D3DRS_CLIPPLANEENABLE, "D3DRS_CLIPPLANEENABLE"},
    {DebugMenuUniqueID(), D3DRS_POINTSIZE, "D3DRS_POINTSIZE"},
    {DebugMenuUniqueID(), D3DRS_POINTSIZE_MIN, "D3DRS_POINTSIZE_MIN"},
    {DebugMenuUniqueID(), D3DRS_POINTSPRITEENABLE, "D3DRS_POINTSPRITEENABLE"},
    {DebugMenuUniqueID(), D3DRS_POINTSCALEENABLE, "D3DRS_POINTSCALEENABLE"},
    {DebugMenuUniqueID(), D3DRS_POINTSCALE_A, "D3DRS_POINTSCALE_A"},
    {DebugMenuUniqueID(), D3DRS_POINTSCALE_B, "D3DRS_POINTSCALE_B"},
    {DebugMenuUniqueID(), D3DRS_POINTSCALE_C, "D3DRS_POINTSCALE_C"},
    {DebugMenuUniqueID(), D3DRS_MULTISAMPLEANTIALIAS, "D3DRS_MULTISAMPLEANTIALIAS"},
    {DebugMenuUniqueID(), D3DRS_MULTISAMPLEMASK, "D3DRS_MULTISAMPLEMASK"},
    {DebugMenuUniqueID(), D3DRS_PATCHEDGESTYLE, "D3DRS_PATCHEDGESTYLE"},
    {DebugMenuUniqueID(), D3DRS_DEBUGMONITORTOKEN, "D3DRS_DEBUGMONITORTOKEN"},
    {DebugMenuUniqueID(), D3DRS_POINTSIZE_MAX, "D3DRS_POINTSIZE_MAX"},
    {DebugMenuUniqueID(), D3DRS_COLORWRITEENABLE, "D3DRS_COLORWRITEENABLE"},
    {DebugMenuUniqueID(), D3DRS_TWEENFACTOR, "D3DRS_TWEENFACTOR"},
    {DebugMenuUniqueID(), D3DRS_POSITIONDEGREE, "D3DRS_POSITIONDEGREE"},
    {DebugMenuUniqueID(), D3DRS_NORMALDEGREE, "D3DRS_NORMALDEGREE"},
    {DebugMenuUniqueID(), D3DRS_SCISSORTESTENABLE, "D3DRS_SCISSORTESTENABLE"},
    {DebugMenuUniqueID(), D3DRS_SLOPESCALEDEPTHBIAS, "D3DRS_SLOPESCALEDEPTHBIAS"},
    {DebugMenuUniqueID(), D3DRS_ANTIALIASEDLINEENABLE, "D3DRS_ANTIALIASEDLINEENABLE"},
    {DebugMenuUniqueID(), D3DRS_MINTESSELLATIONLEVEL, "D3DRS_MINTESSELLATIONLEVEL"},
    {DebugMenuUniqueID(), D3DRS_MAXTESSELLATIONLEVEL, "D3DRS_MAXTESSELLATIONLEVEL"},
    {DebugMenuUniqueID(), D3DRS_ADAPTIVETESS_X, "D3DRS_ADAPTIVETESS_X"},
    {DebugMenuUniqueID(), D3DRS_ADAPTIVETESS_Y, "D3DRS_ADAPTIVETESS_Y"},
    {DebugMenuUniqueID(), D3DRS_ADAPTIVETESS_Z, "D3DRS_ADAPTIVETESS_Z"},
    {DebugMenuUniqueID(), D3DRS_ADAPTIVETESS_W, "D3DRS_ADAPTIVETESS_W"},
    {DebugMenuUniqueID(), D3DRS_ENABLEADAPTIVETESSELLATION, "D3DRS_ENABLEADAPTIVETESSELLATION"},
    {DebugMenuUniqueID(), D3DRS_TWOSIDEDSTENCILMODE, "D3DRS_TWOSIDEDSTENCILMODE"},
    {DebugMenuUniqueID(), D3DRS_CCW_STENCILFAIL, "D3DRS_CCW_STENCILFAIL"},
    {DebugMenuUniqueID(), D3DRS_CCW_STENCILZFAIL, "D3DRS_CCW_STENCILZFAIL"},
    {DebugMenuUniqueID(), D3DRS_CCW_STENCILPASS, "D3DRS_CCW_STENCILPASS"},
    {DebugMenuUniqueID(), D3DRS_CCW_STENCILFUNC, "D3DRS_CCW_STENCILFUNC"},
    {DebugMenuUniqueID(), D3DRS_COLORWRITEENABLE1, "D3DRS_COLORWRITEENABLE1"},
    {DebugMenuUniqueID(), D3DRS_COLORWRITEENABLE2, "D3DRS_COLORWRITEENABLE2"},
    {DebugMenuUniqueID(), D3DRS_COLORWRITEENABLE3, "D3DRS_COLORWRITEENABLE3"},
    {DebugMenuUniqueID(), D3DRS_SRGBWRITEENABLE, "D3DRS_SRGBWRITEENABLE"},
    {DebugMenuUniqueID(), D3DRS_DEPTHBIAS, "D3DRS_DEPTHBIAS"},
    {DebugMenuUniqueID(), D3DRS_WRAP8, "D3DRS_WRAP8"},
    {DebugMenuUniqueID(), D3DRS_WRAP9, "D3DRS_WRAP9"},
    {DebugMenuUniqueID(), D3DRS_WRAP10, "D3DRS_WRAP10"},
    {DebugMenuUniqueID(), D3DRS_WRAP11, "D3DRS_WRAP11"},
    {DebugMenuUniqueID(), D3DRS_WRAP12, "D3DRS_WRAP12"},
    {DebugMenuUniqueID(), D3DRS_WRAP13, "D3DRS_WRAP13"},
    {DebugMenuUniqueID(), D3DRS_WRAP14, "D3DRS_WRAP14"},
    {DebugMenuUniqueID(), D3DRS_WRAP15, "D3DRS_WRAP15"},
  };

  constexpr auto numRenderStates = std::size(renderstates);
  DWORD renderStateValues[numRenderStates]{ 0 };
  for (int i = 0; i < numRenderStates; i++)
  {
    const auto& rs = renderstates[i];
    const auto& renderStateKey = getRenderStateKey(rs);
    renderStateValues[i] = pLLRenderer->GetRenderState(renderStateKey);

    g_DebugMenu.DebugVar("Rendering Debug", getRenderStateName(rs), getRenderStateDebugID(rs), renderStateValues[i]);

    pLLRenderer->SetRenderState(renderStateKey, renderStateValues[i]);
  }
}


void TextureStageStateDebugger::Process(LowlevelRenderer* pLLRenderer, uint32_t pDebugID)
{
  static uint32_t expectedDebugId = 0;
  g_DebugMenu.DebugVar("Rendering Debug", "\n\nTSS Debug ID", DebugMenuUniqueID(), expectedDebugId);

  if (pDebugID != expectedDebugId || expectedDebugId == 0)
  {
    return;
  }

  using TextureStageStateInfo = std::tuple<uint64_t, D3DTEXTURESTAGESTATETYPE, const char*>;
  
  constexpr auto getTextureStateDebugID = [](const TextureStageStateInfo& tss) { return std::get<0>(tss); };
  constexpr auto getTextureStateKey = [](const TextureStageStateInfo& tss) { return std::get<1>(tss); };
  constexpr auto getTextureStateName = [](const TextureStageStateInfo& tss) { return std::get<2>(tss); };
  static const TextureStageStateInfo textureStates[] = {
    {DebugMenuUniqueID(),D3DTSS_COLOROP, "D3DTSS_COLOROP"},
    {DebugMenuUniqueID(),D3DTSS_COLORARG1, "D3DTSS_COLORARG1"},
    {DebugMenuUniqueID(),D3DTSS_COLORARG2, "D3DTSS_COLORARG2"},
    {DebugMenuUniqueID(),D3DTSS_ALPHAOP, "D3DTSS_ALPHAOP"},
    {DebugMenuUniqueID(),D3DTSS_ALPHAARG1, "D3DTSS_ALPHAARG1"},
    {DebugMenuUniqueID(),D3DTSS_ALPHAARG2, "D3DTSS_ALPHAARG2"},
    {DebugMenuUniqueID(),D3DTSS_BUMPENVMAT00, "D3DTSS_BUMPENVMAT00"},
    {DebugMenuUniqueID(),D3DTSS_BUMPENVMAT01, "D3DTSS_BUMPENVMAT01"},
    {DebugMenuUniqueID(),D3DTSS_BUMPENVMAT10, "D3DTSS_BUMPENVMAT10"},
    {DebugMenuUniqueID(),D3DTSS_BUMPENVMAT11, "D3DTSS_BUMPENVMAT11"},
    {DebugMenuUniqueID(),D3DTSS_TEXCOORDINDEX, "D3DTSS_TEXCOORDINDEX"},
    {DebugMenuUniqueID(),D3DTSS_BUMPENVLSCALE, "D3DTSS_BUMPENVLSCALE"},
    {DebugMenuUniqueID(),D3DTSS_BUMPENVLOFFSET, "D3DTSS_BUMPENVLOFFSET"},
    {DebugMenuUniqueID(),D3DTSS_TEXTURETRANSFORMFLAGS, "D3DTSS_TEXTURETRANSFORMFLAGS"},
    {DebugMenuUniqueID(),D3DTSS_COLORARG0, "D3DTSS_COLORARG0"},
    {DebugMenuUniqueID(),D3DTSS_ALPHAARG0, "D3DTSS_ALPHAARG0"},
    {DebugMenuUniqueID(),D3DTSS_RESULTARG, "D3DTSS_RESULTARG"},
    {DebugMenuUniqueID(),D3DTSS_CONSTANT, "D3DTSS_CONSTANT"},
  };

  constexpr auto numTextureStates = std::size(textureStates);
  DWORD renderStateValues[numTextureStates]{ 0 };
  for (int i = 0; i < numTextureStates; i++)
  {
    const auto& rs = textureStates[i];
    const auto& renderStateKey = getTextureStateKey(rs);
    renderStateValues[i] = pLLRenderer->GetTextureStageState(0, renderStateKey);

    g_DebugMenu.DebugVar("Rendering Debug", getTextureStateName(rs), getTextureStateDebugID(rs), renderStateValues[i]);

    pLLRenderer->SetTextureStageState(0, renderStateKey, renderStateValues[i]);
  }
}


void TextureSamplerDebugger::Process(LowlevelRenderer* pLLRenderer, uint32_t pDebugID)
{
  static uint32_t expectedDebugId = 0;
  g_DebugMenu.DebugVar("Rendering Debug", "\n\nTSAMP Debug ID", DebugMenuUniqueID(), expectedDebugId);

  if (pDebugID != expectedDebugId || expectedDebugId == 0)
  {
    return;
  }

  using SamplerStageStateInfo = std::tuple<uint64_t, D3DSAMPLERSTATETYPE, const char*>;

  constexpr auto getSamplerStateDebugID = [](const SamplerStageStateInfo& info) { return std::get<0>(info); };
  constexpr auto getSamplerStateKey = [](const SamplerStageStateInfo& info) { return std::get<1>(info); };
  constexpr auto getSamplerStateName = [](const SamplerStageStateInfo& info) { return std::get<2>(info); };
  static const SamplerStageStateInfo samplerStates[] = {
    {DebugMenuUniqueID(),D3DSAMP_ADDRESSU, "D3DSAMP_ADDRESSU"},
    {DebugMenuUniqueID(),D3DSAMP_ADDRESSV, "D3DSAMP_ADDRESSV"},
    {DebugMenuUniqueID(),D3DSAMP_ADDRESSW, "D3DSAMP_ADDRESSW"},
    {DebugMenuUniqueID(),D3DSAMP_BORDERCOLOR, "D3DSAMP_BORDERCOLOR"},
    {DebugMenuUniqueID(),D3DSAMP_MAGFILTER, "D3DSAMP_MAGFILTER"},
    {DebugMenuUniqueID(),D3DSAMP_MINFILTER, "D3DSAMP_MINFILTER"},
    {DebugMenuUniqueID(),D3DSAMP_MIPFILTER, "D3DSAMP_MIPFILTER"},
    {DebugMenuUniqueID(),D3DSAMP_MIPMAPLODBIAS, "D3DSAMP_MIPMAPLODBIAS"},
    {DebugMenuUniqueID(),D3DSAMP_MAXMIPLEVEL, "D3DSAMP_MAXMIPLEVEL"},
    {DebugMenuUniqueID(),D3DSAMP_MAXANISOTROPY, "D3DSAMP_MAXANISOTROPY"},
    {DebugMenuUniqueID(),D3DSAMP_SRGBTEXTURE, "D3DSAMP_SRGBTEXTURE"},
    {DebugMenuUniqueID(),D3DSAMP_ELEMENTINDEX, "D3DSAMP_ELEMENTINDEX"},
    {DebugMenuUniqueID(),D3DSAMP_DMAPOFFSET, "D3DSAMP_DMAPOFFSET"},
  };

  constexpr auto numSamplerStates = std::size(samplerStates);
  DWORD samplerStateValues[numSamplerStates]{ 0 };
  for (int i = 0; i < numSamplerStates; i++)
  {
    const auto& rs = samplerStates[i];
    const auto& samplerStateKey = getSamplerStateKey(rs);
    samplerStateValues[i] = pLLRenderer->GetSamplerState(0, samplerStateKey);

    g_DebugMenu.DebugVar("Rendering Debug", getSamplerStateName(rs), getSamplerStateDebugID(rs), samplerStateValues[i]);

    pLLRenderer->SetSamplerState(0, samplerStateKey, samplerStateValues[i]);
  }
}
