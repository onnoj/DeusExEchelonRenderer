#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "lightmanager.h"
#include "utils/debugmenu.h"
#include <random>

void LightManager::Initialize(LowlevelRenderer* pLLRenderer)
{
	m_LLRenderer = pLLRenderer;
	m_Effects.insert(std::make_pair(LE_None, &effectNOP));
	m_Effects.insert(std::make_pair(LE_TorchWaver, &effectNOP));
	m_Effects.insert(std::make_pair(LE_FireWaver, &effectNOP));
	m_Effects.insert(std::make_pair(LE_WateryShimmer, &effectNOP));
	m_Effects.insert(std::make_pair(LE_Searchlight, &effectNOP));
	m_Effects.insert(std::make_pair(LE_SlowWave, &effectNOP));
	m_Effects.insert(std::make_pair(LE_FastWave, &effectNOP));
	m_Effects.insert(std::make_pair(LE_CloudCast, &effectNOP));
	m_Effects.insert(std::make_pair(LE_StaticSpot, &effectNOP));
	m_Effects.insert(std::make_pair(LE_Shock, &effectNOP));
	m_Effects.insert(std::make_pair(LE_Disco, &effectDisco));
	m_Effects.insert(std::make_pair(LE_Warp, &effectNOP));
	m_Effects.insert(std::make_pair(LE_Spotlight, &effectNOP));
	m_Effects.insert(std::make_pair(LE_NonIncidence, &effectNOP));
	m_Effects.insert(std::make_pair(LE_Shell, &effectNOP));
	m_Effects.insert(std::make_pair(LE_OmniBumpMap, &effectNOP));
	m_Effects.insert(std::make_pair(LE_Interference, &effectNOP));
	m_Effects.insert(std::make_pair(LE_Cylinder, &effectNOP));
	m_Effects.insert(std::make_pair(LE_Rotor, &effectNOP));
	m_Effects.insert(std::make_pair(LE_Unused, &effectNOP));
	m_KnownLights.reserve(10000);
}

void LightManager::Shutdown()
{
	m_LLRenderer = nullptr;
}

void LightManager::Render(FSceneNode* Frame)
{
	assert(m_LLRenderer != nullptr);
	auto& ctx = *g_ContextManager.GetContext();

	UModel* Model = Frame->Level->Model;
	auto& GSurfs = Model->Surfs;
	auto& GNodes = Model->Nodes;
	auto& GVerts = Model->Verts;
	auto& GVertPoints = Model->Points;

	//We only need to render the lights once per map.
	static uint32_t frameCount = 0;
	frameCount++;

	auto lightOffset = m_LowerBound;
	auto numberOfLights = m_KnownLights.size();
	m_LightIndexOffset = lightOffset;
	check((numberOfLights&0xFFFFF000ul)==0);

	for(auto light : m_KnownLights)
	{
		uint32_t lightIndex = (light->GetIndex() - lightOffset) + uint32_t(ReservedSlots::MAX_RESERVED);
		if (light != nullptr &&
			light->LightType == ELightType::LT_Steady &&
			light->LightBrightness > 0)
		{
			FVector pos = light->Location + FVector(0.0f, 0.0f, (frameCount % 2) ? 0.01f : 0.0f); //keep lights awake
			auto radius = light->WorldLightRadius() * ctx.uservalues.brightness;//((light->WorldLightRadius())/2.0f);
			auto location = light->Location.TransformPointBy(Frame->Coords);
			auto brightness = light->LightBrightness / 255.f;
			//auto effect          = FLightManager::Effects[(Actor->LightEffect<LE_MAX) ? Actor->LightEffect : 0];
			FPlane floatColor{};
			GRender->GlobalLighting((Frame->Viewport->Actor->ShowFlags & SHOW_PlayerCtrl) != 0, light, brightness, floatColor);

			floatColor *= brightness * light->Level->Brightness;
			floatColor.X = min(floatColor.X, 1.0f);
			floatColor.Y = min(floatColor.Y, 1.0f);
			floatColor.Z = min(floatColor.Z, 1.0f);

			D3DLIGHT9 d3dLight{};
			ZeroMemory(&d3dLight, sizeof(d3dLight));
			d3dLight.Position.x = pos.X;
			d3dLight.Position.y = pos.Y;
			d3dLight.Position.z = pos.Z;
			d3dLight.Diffuse.r = floatColor.X;
			d3dLight.Diffuse.g = floatColor.Y;
			d3dLight.Diffuse.b = floatColor.Z;

			//0/1/1 should results in physically correct lighting: // 1/(d*d)
			//however, this results in everything being too dark in remix.
			const bool isDisco = (light->LightEffect == ELightEffect::LE_Disco);
			const bool isSpotlight = (light->LightEffect == ELightEffect::LE_Spotlight || light->LightEffect == ELightEffect::LE_StaticSpot);
			const bool isPointlight = (light->LightEffect == ELightEffect::LE_None);

			if (isPointlight)
			{
				d3dLight.Type = D3DLIGHT_POINT;
				d3dLight.Specular = d3dLight.Diffuse;
				d3dLight.Range = radius;
				d3dLight.Attenuation0 = 0.0f;
				d3dLight.Attenuation1 = 0.0f;
				d3dLight.Attenuation2 = 0.0f;
			}
			else if (isSpotlight || isDisco)
			{
				FVector direction = light->GetViewRotation().Vector();
				float sine = 1.0 - light->LightCone / 256.0;
				float rSine = 1.0 / (1.0 - sine);
				float sineRSine = sine * rSine;
				float sineSq = sine * sine;

				float thetaBoost = 1.0f;
				float phiBoost = 1.0f;
				float rangeBoost = 1.0f;
				g_DebugMenu.DebugVar("Lighting", "thetaBoost", DebugMenuUniqueID(), thetaBoost, {DebugMenuValueOptions::editor::slider, 1.0f, 50.0f});
				g_DebugMenu.DebugVar("Lighting", "phiBoost", DebugMenuUniqueID(), phiBoost, {DebugMenuValueOptions::editor::slider, 1.0f, 50.0f});
				g_DebugMenu.DebugVar("Lighting", "rangeBoost", DebugMenuUniqueID(), rangeBoost, {DebugMenuValueOptions::editor::slider, 1.0f, 50.0f});

				d3dLight.Type = D3DLIGHT_SPOT;
				d3dLight.Range = light->WorldLightRadius() * rangeBoost;
				d3dLight.Diffuse.r *= (d3dLight.Range) * rangeBoost;
				d3dLight.Diffuse.g *= (d3dLight.Range) * rangeBoost;
				d3dLight.Diffuse.b *= (d3dLight.Range) * rangeBoost;
				d3dLight.Direction = { direction.X, direction.Y, direction.Z };
				d3dLight.Theta = sine * thetaBoost;
				d3dLight.Phi = sine * phiBoost;
			}
			else
			{
				continue;
			}

			if (g_options.hasLights)// && ((currentLevel!=lastLevel) || (frameCount%60)==0))
			{
				m_Effects[ELightEffect(light->LightEffect)](lightIndex, light, d3dLight);
				m_LLRenderer->RenderLight(lightIndex, d3dLight);

				if (isDisco)
				{
					d3dLight.Range *= 2.0f;
					uint32_t temporaryIndex = lightIndex << 4;
					for (uint8_t i = 0; i < 16; i++)
					{
						m_Effects[ELightEffect(light->LightEffect)](temporaryIndex + i, light, d3dLight);
						m_LLRenderer->RenderLight(temporaryIndex + i, d3dLight);
					}
				}
			}
		}
	}

	///JCDenton's spotlight vision augmentation
	{
		const bool lightEnabled = (m_lightAugmentation != nullptr && m_lightAugmentation->bIsActive);
		auto fwd = -(Frame->Coords.XAxis ^ Frame->Coords.YAxis);
		D3DLIGHT9 d3dLight{};
		ZeroMemory(&d3dLight, sizeof(d3dLight));
		d3dLight.Position.x = Frame->Coords.Origin.X;
		d3dLight.Position.y = Frame->Coords.Origin.Y;
		d3dLight.Position.z = Frame->Coords.Origin.Z;
		d3dLight.Diffuse.r = 0.7f;
		d3dLight.Diffuse.g = 0.7f;
		d3dLight.Diffuse.b = 1.0f;
		d3dLight.Type = D3DLIGHT_SPOT;
		d3dLight.Range = lightEnabled ? 5000.0f : 0.0f;
		//d3dLight.Diffuse.r *= (d3dLight.Range);
		//d3dLight.Diffuse.g *= (d3dLight.Range);
		//d3dLight.Diffuse.b *= (d3dLight.Range);
		d3dLight.Direction = { fwd.X, fwd.Y, fwd.Z };
		d3dLight.Theta = 1.0f;
		d3dLight.Phi = 1.0f;
		m_LLRenderer->RenderLight((uint32_t)ReservedSlots::jcDentonLight, d3dLight);
	}

	/// 
	m_LLRenderer->FlushLights();
}

void LightManager::CacheLights()
{
	auto& ctx = *g_ContextManager.GetContext();

	UModel* Model = ctx.frameSceneNode->Level->Model;
	auto& GSurfs = Model->Surfs;
	auto& GNodes = Model->Nodes;
	auto& GVerts = Model->Verts;
	auto& GVertPoints = Model->Points;

	std::unordered_set<void*> seenLights; 
	seenLights.reserve(Model->Lights.Num());

	for (int i = 0; i < Model->Lights.Num(); i++)
	{
		auto light = Model->Lights(i);

		//Skip if we've already seen this light.
		if (light==nullptr || !seenLights.insert(light).second)
		{
			continue;
		}

		m_LowerBound = min(m_LowerBound, light->GetIndex());
		m_UpperBound = max(m_UpperBound, light->GetIndex());
		m_KnownLights.push_back(light);
	}
}

void LightManager::OnLevelChange()
{
	m_LowerBound = 0xFFFFFFFFul;
	m_UpperBound = 0x00000000ul;
	m_LightIndexOffset.reset();
	m_lightAugmentation = nullptr;
	m_KnownLights.clear();

	CacheLights();

	FObjectIterator it(AAugmentationManager::StaticClass());
	for (; it.operator int() != 0; it.operator++())
	{
		AAugmentationManager* manager = static_cast<AAugmentationManager*>(*it);
		for (auto augmentation = manager->FirstAug; augmentation != nullptr; augmentation = augmentation->Next)
		{
			if (augmentation->GetClass()->FriendlyName.GetIndex() == 4423)
			{
				m_lightAugmentation = augmentation;
				break;
			}
		}
	}
}

///

void LightManager::effectDisco(uint32_t pLightIndex, AActor* pLight, D3DLIGHT9& outLight)
{
	std::mt19937 mt(pLightIndex);
	std::uniform_real_distribution urd(0.0f, float(PI)*2.0f);

	auto& ctx = *g_ContextManager.GetContext();
	auto levelInfo = ctx.frameSceneNode->Level->GetLevelInfo();
	float scaleY = (std::sinf(urd(mt) + (levelInfo->TimeSeconds * 0.50f)));
	float scaleP = (std::sinf(urd(mt) + (levelInfo->TimeSeconds * 0.50f)));
	float scaleR = (std::sinf(urd(mt) + (levelInfo->TimeSeconds * 0.50f)));

	D3DXMATRIX m;
	D3DXMatrixRotationYawPitchRoll(&m, scaleY, scaleP, scaleR);
	D3DXVec3TransformCoord((D3DXVECTOR3*)&outLight.Direction, (D3DXVECTOR3*)&outLight.Direction, &m);

	g_DebugMenu.DebugVar("Lighting", "Disco Direction X", DebugMenuUniqueID(), outLight.Direction.x, {DebugMenuValueOptions::editor::slider, -3.16f, 3.16f});
	g_DebugMenu.DebugVar("Lighting", "Disco Direction Y", DebugMenuUniqueID(), outLight.Direction.y, {DebugMenuValueOptions::editor::slider, -3.16f, 3.16f});
	g_DebugMenu.DebugVar("Lighting", "Disco Direction Z", DebugMenuUniqueID(), outLight.Direction.z, {DebugMenuValueOptions::editor::slider, -3.16f, 3.16f});
}