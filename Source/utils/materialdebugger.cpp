#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include <locale>
#include <codecvt>
#include <filesystem>
#include <codecvt> // codecvt_utf8
#include <locale>  // wstring_convert

#include "uefacade.h"
#include "hacks/misc.h"
#include "utils/debugmenu.h"
#include "materialdebugger.h"
#include "rendering/llrenderer.h"
#include "rendering/hlrenderer.h"

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

void MaterialDebugger::Update(FSceneNode* Frame)
{
	{
		
	}
	///
	FVector dir = -(Frame->Coords.XAxis ^ Frame->Coords.YAxis);
	FVector begin = Frame->Coords.Origin;
	FVector end = Frame->Coords.Origin + dir*1000000.0f;
	FCheckResult res(0.0f);
	auto levelModel = Frame->Level->Model;
	UETextureType textureID = 0;
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
		for(auto raycast = &res; raycast != nullptr; raycast = raycast->GetNext())
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
				actorName = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(raycast->Actor->GetFullName());
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
			::sprintf(&b[0], "%08llx", t->remixHash);
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
			if ((node->NodeFlags & nf.first)!=0)
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
		if ((surfFlags & pf.first)==pf.first)
		{
			if (!surfaceFlagTxt.empty())
				surfaceFlagTxt += "\r\n";
			surfaceFlagTxt += pf.second;
			hasSurfaceFlags = true;
		}

		if ((textureFlags & pf.first)==pf.first)
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
		for(auto cls = actor->GetClass(); cls != nullptr; cls = cls->GetSuperClass())
		{
			actorClassPath += std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(cls->GetNameCPP());
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
	g_DebugMenu.DebugVar("Modding - Inspector", "Viewed Texture", DebugMenuUniqueID(), textureID);
	g_DebugMenu.DebugVar("Modding - Inspector", "Node Flags", DebugMenuUniqueID(), nodeFlagsTxt);
	g_DebugMenu.DebugVar("Modding - Inspector", "Surface Flags", DebugMenuUniqueID(), surfaceFlagTxt);
	g_DebugMenu.DebugVar("Modding - Inspector", "Texture Flags", DebugMenuUniqueID(), textureFlagTxt);
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

void MaterialDebugger::renderAllTextures()
{
	auto facade = static_cast<UD3D9FPRenderDevice*>(GRenderDevice);
	auto llRenderer = facade->GetLLRenderer();
	auto hlRenderer = facade->GetHLRenderer();
	auto& textureManager = hlRenderer->GetTextureManager();



#if 1
	//dump currently loaded textures
	uint32_t count = 0;
	int row = 0;
	int col = 0;

	for (FObjectIterator it = FObjectIterator(UTexture::StaticClass()); *it!=nullptr; it.operator++())
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
			auto ctx = g_ContextManager.GetContext();
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
			hlRenderer->Draw3DCube(ctx->frameSceneNode, widgetPos, md, 10.0f);
		}
		texture->Unlock(textureInfo);
	}
	GLog->Logf(L"[EchelonRenderer]\t Rendered %d textures.", count);
#endif
}

void MaterialDebugger::exportHashMappings(bool pLoadAllTextures)
{
	auto facade = static_cast<UD3D9FPRenderDevice*>(GRenderDevice);
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
	

	auto convertWStringToUTF8 = [](const wchar_t* pWString){
		static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
		return utf8_conv.to_bytes(pWString);
	};

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
		{
			textureManager.BindTexture(texture->PolyFlags, md);
			hlRenderer->SetWorldTransformStateToIdentity();
			static LowlevelRenderer::VertexPos3Tex0to4 vbuffer;
			llRenderer->RenderTriangleList( &vbuffer, 1, 1, 0, 0);
		}

		if (textureInfo.Texture != nullptr)
		{
			auto textureObject = json::object();
			textureObject["package"] = convertWStringToUTF8(texturePackage->GetPathName()).c_str();
			textureObject["path"] = convertWStringToUTF8(textureInfo.Texture->GetPathName()).c_str();
			textureObject["name"] = convertWStringToUTF8(textureInfo.Texture->GetName()).c_str();
			textureObject["cacheid"] = textureInfo.CacheID;
			textureObject["index"] = textureInfo.Texture->GetIndex();
			textureObject["remixhash"] = md->remixHash;
			textureObject["flags"] = json::array();
			textureObject["realtime"] = (textureInfo.bRealtime!=0);
			for (const auto& flag : knownPolyFlags)
			{
				if ((textureInfo.Texture->PolyFlags & flag.first) != 0)
				{
					textureObject["flags"].push_back(flag.second);
				}
			}

			char remixHashHex[64]{ 0 };
			::sprintf(&remixHashHex[0], "%llx", md->remixHash);
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