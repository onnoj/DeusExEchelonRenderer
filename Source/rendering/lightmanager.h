#pragma once

//included in PCH
//#include <deusex/Extension/Inc/ExtensionCore.h>
//#include <deusex/Extension/Inc/ExtPlayerPawn.h>
//#include <deusex/DeusEx/Inc/DeusEx.h>

class LightManager
{
public:
  void Initialize(LowlevelRenderer* pLLRenderer);
  void Shutdown();
  void Render(FSceneNode* Frame);
  void OnLevelChange();
protected:
  void CacheLights();
  static void effectNOP(uint32_t pLightIndex, AActor* pLight, D3DLIGHT9& outLight) {};
  static void effectDisco(uint32_t pLightIndex, AActor* pLight, D3DLIGHT9& outLight);
protected:
  enum class ReservedSlots
  {
    jcDentonLight = 0,
    MAX_RESERVED
  };
private:
  LowlevelRenderer* m_LLRenderer = nullptr;
  std::unordered_map<ELightEffect, std::function<void(uint32_t pLightIndex, AActor* pLight, D3DLIGHT9& outLight)>> m_Effects;
  uint32_t m_LowerBound = 0xFFFFFFFFul;
  uint32_t m_UpperBound = 0x00000000ul;
  std::optional<uint32_t> m_LightIndexOffset;
  AAugmentation* m_lightAugmentation = nullptr;
  std::vector<AActor*> m_KnownLights;
};