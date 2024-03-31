#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include <locale>
#include <codecvt>

#include "uefacade.h"
#include "hacks/misc.h"
#include "utils/debugmenu.h"
#include "materialdebugger.h"
MaterialDebugger g_MaterialDebugger;



void MaterialDebugger::Update(FSceneNode* Frame)
{
	FVector dir = -(Frame->Coords.XAxis ^ Frame->Coords.YAxis);
	FVector begin = Frame->Coords.Origin;
	FVector end = Frame->Coords.Origin + dir*1000000.0f;
	FCheckResult res(0.0f);
	auto levelModel = Frame->Level->Model;
	UETextureType textureID = 0;
	UnrealPolyFlags surfFlags = 0;
	UnrealPolyFlags textureFlags = 0;
	FBspNode* node = nullptr;
	uint32_t iNode = 0;
	uint32_t iActor = 0;
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
							FTextureInfo info{};
							texture->Lock(info, 0, 0, GRenderDevice);
							textureID = info.CacheID;
							surfFlags = surf.PolyFlags;
							textureFlags = texture->PolyFlags;
							g_DebugMenu.VisitTexture(&info);
							texture->Unlock(info);
							break;
						}
					}
				}
			}
			else if (raycast->Actor)
			{
				iActor = raycast->Actor->GetIndex();
				actorName = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(raycast->Actor->GetFullName());
				UTexture* texture = nullptr;
				if (raycast->Actor->Mesh != nullptr)
				{
					texture = raycast->Actor->Mesh->GetTexture(0, raycast->Actor);
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
			if (remixTextureHashes.find(&b[0]) < 0)
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

	g_DebugMenu.DebugVar("Rendering", "iNode", DebugMenuUniqueID(), iNode);
	g_DebugMenu.DebugVar("Rendering", "iSurf", DebugMenuUniqueID(), iSurf);
	g_DebugMenu.DebugVar("Rendering", "Remix Hashes", DebugMenuUniqueID(), remixTextureHashes);
	g_DebugMenu.DebugVar("Rendering", "Actor Index", DebugMenuUniqueID(), iActor);
	g_DebugMenu.DebugVar("Rendering", "Actor Name", DebugMenuUniqueID(), actorName);
	g_DebugMenu.DebugVar("Rendering", "Viewed Texture", DebugMenuUniqueID(), textureID);
	g_DebugMenu.DebugVar("Rendering", "Node Flags", DebugMenuUniqueID(), nodeFlagsTxt);
	g_DebugMenu.DebugVar("Rendering", "Surface Flags", DebugMenuUniqueID(), surfaceFlagTxt);
	g_DebugMenu.DebugVar("Rendering", "Texture Flags", DebugMenuUniqueID(), textureFlagTxt);
}