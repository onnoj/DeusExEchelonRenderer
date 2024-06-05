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
	m_LevelLights.reserve(10000);
}

void LightManager::Shutdown()
{
	m_LLRenderer = nullptr;
}

void LightManager::Update(FSceneNode* Frame)
{
	assert(m_LLRenderer != nullptr);
	auto& ctx = *g_ContextManager.GetContext();


}

void LightManager::Render(FSceneNode* Frame)
{
	Update(Frame); //move to hlrenderer
	//
	assert(m_LLRenderer != nullptr);
	auto& ctx = *g_ContextManager.GetContext();

	//Mark lights that were rendered in the previous frame, but not this frame, as inactive.
	//push to renderer, and then wipe the list for use for the next frame.
	for (auto it = m_ExpiringFrameLights.begin(); it != m_ExpiringFrameLights.end();)
	{
		LightInfo lightInfo = *it;
		if (HasFrameLight(lightInfo.lightActor))
		{
			it = m_ExpiringFrameLights.erase(it);
		}
		else
		{
			it++;
		}

	}

	auto renderLight = [&](LightInfo& light)
	{
		m_Effects[ELightEffect(light.lightActor->LightEffect)](*light.lightIndex, light.lightActor, light.d3dLight);
		m_LLRenderer->RenderLight(*light.lightIndex, light.d3dLight);

		if (light.isDisco)
		{
			light.d3dLight.Range *= 2.0f;
			for (uint8_t i = 0; i < light.lightExtraCount; i++)
			{
				uint32_t discoReservedIndex = LevelLightsExtraRange.lower + light.lightExtraIndex + i;
				m_Effects[ELightEffect(light.lightActor->LightEffect)](discoReservedIndex, light.lightActor, light.d3dLight);
				m_LLRenderer->RenderLight(discoReservedIndex, light.d3dLight);
			}
		}
	};

	//Render all static level lights
	for(auto& light : m_LevelLights)
	{
		CalculateLightInfo(light.lightActor, light);
		renderLight(light);
	}
	for(auto& light : m_FrameLights)
	{
		renderLight(light);
	}
	for(auto& light : m_ExpiringFrameLights)
	{
		check(light.lightIndex);
		m_LLRenderer->DisableLight(*light.lightIndex);
	}

	///JCDenton's spotlight vision augmentation
	{
		const uint32_t lightIndex = ReservedLightsRange.lower + (uint32_t)ReservedSlots::jcDentonLight;
		const bool lightEnabled = (m_lightAugmentation != nullptr && m_lightAugmentation->bIsActive);
		auto fwd = -(Frame->Coords.XAxis ^ Frame->Coords.YAxis);
		D3DLIGHT9 d3dLight{};
		ZeroMemory(&d3dLight, sizeof(d3dLight));
#if defined(CONVERT_TO_LEFTHANDED_COORDINATES) && CONVERT_TO_LEFTHANDED_COORDINATES==1
		d3dLight.Position.x = -Frame->Coords.Origin.X;
		d3dLight.Position.y = Frame->Coords.Origin.Y;
		d3dLight.Position.z = Frame->Coords.Origin.Z;
#else
		d3dLight.Position.x = Frame->Coords.Origin.X;
		d3dLight.Position.y = Frame->Coords.Origin.Y;
		d3dLight.Position.z = Frame->Coords.Origin.Z;
#endif
		d3dLight.Diffuse.r = 0.7f;
		d3dLight.Diffuse.g = 0.7f;
		d3dLight.Diffuse.b = 1.0f;
		d3dLight.Type = D3DLIGHT_SPOT;
		d3dLight.Range = lightEnabled ? 5000.0f : 0.0f;
		//d3dLight.Diffuse.r *= (d3dLight.Range);
		//d3dLight.Diffuse.g *= (d3dLight.Range);
		//d3dLight.Diffuse.b *= (d3dLight.Range);
#if defined(CONVERT_TO_LEFTHANDED_COORDINATES) && CONVERT_TO_LEFTHANDED_COORDINATES==1
		d3dLight.Direction = { -fwd.X, fwd.Y, fwd.Z };
#else
		d3dLight.Direction = { fwd.X, fwd.Y, fwd.Z };
#endif
		d3dLight.Theta = 1.0f;
		d3dLight.Phi = 1.0f;

		D3DXVECTOR3 positionOffset{0.0f, 0.0f, 0.0f};
		D3DXVECTOR3 directionOffset{0.0f, 0.0f, 0.0f};
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight diffuse R", DebugMenuUniqueID(), d3dLight.Diffuse.r, {DebugMenuValueOptions::editor::slider, 0.0f, 1.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight diffuse G", DebugMenuUniqueID(), d3dLight.Diffuse.g, {DebugMenuValueOptions::editor::slider, 0.0f, 1.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight diffuse B", DebugMenuUniqueID(), d3dLight.Diffuse.b, {DebugMenuValueOptions::editor::slider, 0.0f, 1.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight diffuse A", DebugMenuUniqueID(), d3dLight.Diffuse.a, {DebugMenuValueOptions::editor::slider, 0.0f, 1.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight specular R", DebugMenuUniqueID(), d3dLight.Specular.r, {DebugMenuValueOptions::editor::slider, 0.0f, 1.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight specular G", DebugMenuUniqueID(), d3dLight.Specular.g, {DebugMenuValueOptions::editor::slider, 0.0f, 1.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight specular B", DebugMenuUniqueID(), d3dLight.Specular.b, {DebugMenuValueOptions::editor::slider, 0.0f, 1.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight specular A", DebugMenuUniqueID(), d3dLight.Specular.a, {DebugMenuValueOptions::editor::slider, 0.0f, 1.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight position X", DebugMenuUniqueID(), positionOffset.x, {DebugMenuValueOptions::editor::slider, -100.0f, 100.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight position Y", DebugMenuUniqueID(), positionOffset.y, {DebugMenuValueOptions::editor::slider, -100.0f, 100.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight position Z", DebugMenuUniqueID(), positionOffset.z, {DebugMenuValueOptions::editor::slider, -100.0f, 100.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight direction X", DebugMenuUniqueID(), directionOffset.x, {DebugMenuValueOptions::editor::slider, -3.16f, 3.16f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight direction Y", DebugMenuUniqueID(), directionOffset.y, {DebugMenuValueOptions::editor::slider, -3.16f, 3.16f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight direction Z", DebugMenuUniqueID(), directionOffset.z, {DebugMenuValueOptions::editor::slider, -3.16f, 3.16f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight range", DebugMenuUniqueID(), d3dLight.Range, {DebugMenuValueOptions::editor::slider, 0.0f, 5000.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight falloff", DebugMenuUniqueID(), d3dLight.Falloff, {DebugMenuValueOptions::editor::slider, 0.0f, 5000.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight Attenuation0", DebugMenuUniqueID(), d3dLight.Attenuation0, {DebugMenuValueOptions::editor::slider, 0.0f, 2.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight Attenuation1", DebugMenuUniqueID(), d3dLight.Attenuation1, {DebugMenuValueOptions::editor::slider, 0.0f, 2.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight Attenuation2", DebugMenuUniqueID(), d3dLight.Attenuation2, {DebugMenuValueOptions::editor::slider, 0.0f, 2.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight theta", DebugMenuUniqueID(), d3dLight.Theta, {DebugMenuValueOptions::editor::slider, 0.0f, 50.0f});
		g_DebugMenu.DebugVar("Lighting", "JCSpotlight phi", DebugMenuUniqueID(), d3dLight.Phi, {DebugMenuValueOptions::editor::slider, 0.0f, 50.0f});
		d3dLight.Position = D3DXVECTOR3(d3dLight.Position) + positionOffset;
		d3dLight.Direction = D3DXVECTOR3(d3dLight.Direction) + directionOffset;

		m_LLRenderer->RenderLight(lightIndex, d3dLight);
	}

	// send lights to renderer
	m_LLRenderer->FlushLights(); 

	// move left over previous frame lights into recycling for re-use.
	m_ExpiredFrameLights.insert(m_ExpiredFrameLights.end(), m_ExpiringFrameLights.begin(), m_ExpiringFrameLights.end());
	m_ExpiringFrameLights = m_FrameLights;
	m_FrameLights.clear();
}

bool LightManager::CalculateLightInfo(AActor* pActor, LightManager::LightInfo& pmInfo)
{
	auto& ctx = *g_ContextManager.GetContext();
  if (pActor == nullptr ||
    pActor->bSpecialLit ||
    pActor->LightType != ELightType::LT_Steady ||
    pActor->LightBrightness <= 0)
  {
    return false;
  }

	check(pmInfo.lightIndex);
	pmInfo.lightActor = pActor;

	static uint32_t frameCount = 0;
	frameCount++;
  FVector pos = pActor->Location + FVector(0.0f, 0.0f, (frameCount % 2) ? 0.01f : 0.0f);
  auto radius = pActor->WorldLightRadius() * ctx.uservalues.brightness;
  auto brightness = pActor->LightBrightness / 255.f;
  FPlane floatColor{};
  GRender->GlobalLighting((ctx.frameSceneNode->Viewport->Actor->ShowFlags & SHOW_PlayerCtrl) != 0, pActor, brightness, floatColor);
	
  floatColor *= brightness * pActor->Level->Brightness;
  floatColor.X = min(floatColor.X, 1.0f);
  floatColor.Y = min(floatColor.Y, 1.0f);
  floatColor.Z = min(floatColor.Z, 1.0f);

	D3DLIGHT9& d3dLight = pmInfo.d3dLight;
  ZeroMemory(&d3dLight, sizeof(d3dLight));
#if defined(CONVERT_TO_LEFTHANDED_COORDINATES) && CONVERT_TO_LEFTHANDED_COORDINATES==1
  d3dLight.Position.x = -pos.X;
  d3dLight.Position.y = pos.Y;
  d3dLight.Position.z = pos.Z;
#else
  d3dLight.Position.x = pos.X;
  d3dLight.Position.y = pos.Y;
  d3dLight.Position.z = pos.Z;
#endif
  d3dLight.Diffuse.r = floatColor.X;
  d3dLight.Diffuse.g = floatColor.Y;
  d3dLight.Diffuse.b = floatColor.Z;

  d3dLight.Attenuation0 = 1.0f;
  d3dLight.Attenuation1 = 0.0f;
  d3dLight.Attenuation2 = 0.0f;

	pmInfo.isDisco = (pActor->LightEffect == ELightEffect::LE_Disco);
  pmInfo.isSpotlight = (pActor->LightEffect == ELightEffect::LE_Spotlight || pActor->LightEffect == ELightEffect::LE_StaticSpot);
	pmInfo.isPointlight = !pmInfo.isSpotlight && !pmInfo.isDisco; //(pActor->LightEffect == ELightEffect::LE_None || pActor->LightEffect == ELightEffect::LE_NonIncidence);

  if (pmInfo.isPointlight)
  {
    d3dLight.Type = D3DLIGHT_POINT;
    d3dLight.Specular = d3dLight.Diffuse;
    d3dLight.Range = radius;

		/*
		* It seems that the original attenuation in UE was linear with a direct cutoff.
		* D3D9's attenuation formula is Atten = 1/( att0i + att1i * d + att2i * d²).
		* The following is probably mathemathically bullshit, but, it seems to best approach
		* the original lighting formula.
		*/
		d3dLight.Attenuation0 = 0.5f;
		d3dLight.Attenuation1 = 1.0f/radius;
		d3dLight.Attenuation2 = 0.0f;

		g_DebugMenu.DebugVar("Lighting", "pointlight Attenuation0", DebugMenuUniqueID(), d3dLight.Attenuation0);
		g_DebugMenu.DebugVar("Lighting", "pointlight Attenuation1", DebugMenuUniqueID(), d3dLight.Attenuation1);
		g_DebugMenu.DebugVar("Lighting", "pointlight Attenuation2", DebugMenuUniqueID(), d3dLight.Attenuation2);

  }
  else if (pmInfo.isSpotlight || pmInfo.isDisco)
  {
    FVector direction = pActor->GetViewRotation().Vector();
    float sine = 1.0 - pActor->LightCone / 256.0;
    float rSine = 1.0 / (1.0 - sine);
    float sineRSine = sine * rSine;
    float sineSq = sine * sine;

    float thetaBoost = 1.0f;
    float phiBoost = 1.0f;
    float rangeBoost = 1.0f;
    g_DebugMenu.DebugVar("Lighting", "global spotlight thetaBoost", DebugMenuUniqueID(), thetaBoost, { DebugMenuValueOptions::editor::slider, 1.0f, 50.0f });
    g_DebugMenu.DebugVar("Lighting", "global spotlight phiBoost", DebugMenuUniqueID(), phiBoost, { DebugMenuValueOptions::editor::slider, 1.0f, 50.0f });
    g_DebugMenu.DebugVar("Lighting", "global spotlight rangeBoost", DebugMenuUniqueID(), rangeBoost, { DebugMenuValueOptions::editor::slider, 1.0f, 50.0f });

    d3dLight.Type = D3DLIGHT_SPOT;
    d3dLight.Range = pActor->WorldLightRadius() * rangeBoost;
    d3dLight.Diffuse.r *= (d3dLight.Range) * rangeBoost;
    d3dLight.Diffuse.g *= (d3dLight.Range) * rangeBoost;
    d3dLight.Diffuse.b *= (d3dLight.Range) * rangeBoost;
    d3dLight.Direction = { direction.X, direction.Y, direction.Z };
    d3dLight.Theta = sine * thetaBoost;
    d3dLight.Phi = sine * phiBoost;

		g_DebugMenu.DebugVar("Lighting", "spotlight Attenuation0", DebugMenuUniqueID(), d3dLight.Attenuation0);
		g_DebugMenu.DebugVar("Lighting", "spotlight Attenuation1", DebugMenuUniqueID(), d3dLight.Attenuation1);
		g_DebugMenu.DebugVar("Lighting", "spotlight Attenuation2", DebugMenuUniqueID(), d3dLight.Attenuation2);
  }
  else
  {
    return false;
  }


	float diffuseBoost = 1.0f;
	float rangeMultiplier = 1.0f;
	float rangeOffset = 0.0f;
	g_DebugMenu.DebugVar("Lighting", "global light diffuseBoost", DebugMenuUniqueID(), diffuseBoost, { DebugMenuValueOptions::editor::slider, 0.0f, 10.0f });
	g_DebugMenu.DebugVar("Lighting", "global range offset", DebugMenuUniqueID(), rangeOffset, { DebugMenuValueOptions::editor::slider, 0.0f, 1000.0f });
	g_DebugMenu.DebugVar("Lighting", "global range multiplier", DebugMenuUniqueID(), rangeMultiplier, { DebugMenuValueOptions::editor::slider, 0.0f, 50.0f });

	d3dLight.Diffuse.r *= diffuseBoost;
	d3dLight.Diffuse.g *= diffuseBoost;
	d3dLight.Diffuse.b *= diffuseBoost;
	d3dLight.Range *= rangeMultiplier;
	d3dLight.Range += rangeOffset;
	return true;
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
		if (light != nullptr)
		{
			m_LevelLightsLowerBound = min(m_LevelLightsLowerBound, light->GetIndex());
			m_LevelLightsUpperBound = max(m_LevelLightsUpperBound, light->GetIndex());
		}
	}

	for (int i = 0; i < Model->Lights.Num(); i++)
	{
		auto light = Model->Lights(i);

		//Skip if we've already seen this light.
		if (light==nullptr || !seenLights.insert(light).second)
		{
			continue;
		}

		m_LevelLightsLowerBound = min(m_LevelLightsLowerBound, light->GetIndex());
		m_LevelLightsUpperBound = max(m_LevelLightsUpperBound, light->GetIndex());

		LightInfo lightInfo;
		lightInfo.lightIndex = LevelLightsRange.lower + (light->GetIndex() - m_LevelLightsLowerBound);
		check(lightInfo.lightIndex && (*lightInfo.lightIndex >= LevelLightsRange.lower) && (*lightInfo.lightIndex < LevelLightsRange.upper));
		if (CalculateLightInfo(light, lightInfo))
		{
			if (lightInfo.isDisco)
			{
				lightInfo.lightExtraIndex = m_FreeLevelLightsExtraIndex;
				lightInfo.lightExtraCount = 16;
				m_FreeLevelLightsExtraIndex += 16;
				m_FreeLevelLightsExtraIndex %= (LevelLightsExtraRange.upper - LevelLightsExtraRange.lower);
			}
			m_LevelLights.push_back(lightInfo);
		}
	}

	//Find JC's flashlight
	ADeusExPlayer* player = Cast<ADeusExPlayer>(ctx.frameSceneNode->Viewport->Actor);
	for (auto augmentation = player->AugmentationSystem->FirstAug; m_lightAugmentation == nullptr && augmentation != nullptr; augmentation = augmentation->Next)
	{
		std::wstring augName = *(augmentation->GetClass()->FriendlyName);
		if (augName == L"AugLight")
		{
			m_lightAugmentation = augmentation;
			break;
		}
	}
}

void LightManager::OnLevelChange()
{
	if (m_LLRenderer)
	{
		auto disableOldLight = [&](const LightInfo& pLightInfo)
		{
			m_LLRenderer->DisableLight(*pLightInfo.lightIndex);
			for (int i = 0; i < pLightInfo.lightExtraCount; i++)
			{
				uint32_t index = LevelLightsExtraRange.lower + pLightInfo.lightExtraIndex + i;
				m_LLRenderer->DisableLight(index);
			}
		};
		
		for (const auto& l : m_LevelLights)
		{
			disableOldLight(l);
		}
		for (const auto& l : m_FrameLights)
		{
			disableOldLight(l);
		}
		for (const auto& l : m_ExpiringFrameLights)
		{
			disableOldLight(l);
		}
		for (const auto& l : m_ExpiredFrameLights)
		{
			disableOldLight(l);
		}
	}

	m_LevelLightsLowerBound = 0xFFFFFFFFul;
	m_LevelLightsUpperBound = 0x00000000ul;
	m_FreeFrameLightIndex = 0;
	m_FreeLevelLightsExtraIndex = 0;
	m_lightAugmentation = nullptr;
	m_LevelLights.clear();
	m_FrameLights.clear();
	m_ExpiringFrameLights.clear();
	m_ExpiredFrameLights.clear();
	CacheLights();
}

void LightManager::AddFrameLight(AActor* pLight, FVector* pLocation /*= nullptr*/)
{
	LightInfo& lightInfo = [&]() -> LightInfo& {
		for (auto& existingLight : m_FrameLights)
		{
			if (existingLight.lightActor == pLight)
			{
				return existingLight;
			}
		}

		for (auto it = m_ExpiringFrameLights.begin(); it != m_ExpiringFrameLights.end(); it++)
		{
			LightManager::LightInfo frameLightInfo = (*it);
			if (frameLightInfo.lightActor == pLight)
			{
				m_ExpiringFrameLights.erase(it);
				return m_FrameLights.emplace_back(std::move(frameLightInfo));
			}
		}

		for (auto it = m_ExpiredFrameLights.begin(); it != m_ExpiredFrameLights.end(); it++)
		{
			LightManager::LightInfo frameLightInfo = (*it);
			if (frameLightInfo.lightActor == pLight)
			{
				m_ExpiredFrameLights.erase(it);
				return m_FrameLights.emplace_back(std::move(frameLightInfo));
			}
		}
		
		auto& info = m_FrameLights.emplace_back();
		info.lightActor = pLight;
		info.lightIndex = DynamicLightsRange.lower + m_FreeFrameLightIndex;
		m_FreeFrameLightIndex++;
		return info;
	}();

	if (!CalculateLightInfo(pLight, lightInfo))
	{
		m_ExpiringFrameLights.push_back(lightInfo);
	}
}

bool LightManager::HasFrameLight(AActor* pLight)
{
	for (const auto l : m_FrameLights)
	{
		if (l.lightActor == pLight)
		{
			return true;
		}
	}
	return false;
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