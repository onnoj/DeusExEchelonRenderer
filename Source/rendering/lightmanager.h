#pragma once

//included in PCH
//#include <deusex/Extension/Inc/ExtensionCore.h>
//#include <deusex/Extension/Inc/ExtPlayerPawn.h>
//#include <deusex/DeusEx/Inc/DeusEx.h>

class LightManager
{
  enum class LightAllocationTypes
  {
    unknown,
    level,
    reserved,
    dynamic
  };
  struct Range { uint32_t lower = 0; uint32_t upper = 0; };
  struct LightInfo
  { 
    D3DLIGHT9 d3dLight{};
    AActor* lightActor = nullptr; 
    std::optional<uint32_t> lightIndex;
    uint32_t lightExtraIndex = 0;
    uint32_t lightExtraCount = 0;
    bool isDisco = false;
    bool isSpotlight = false;
    bool isPointlight = false;
    void Reset() { d3dLight = {}; lightActor = nullptr; isDisco = isSpotlight = isPointlight = false; }
  };
  static constexpr Range LevelLightsRange = { 1, 125000 };
  static constexpr Range LevelLightsExtraRange = { 125000, 150000 };
  static constexpr Range ReservedLightsRange = { 150000, 200000 };
  static constexpr Range DynamicLightsRange = { 200000, 0xFFFFFFFFul };
public:
  void Initialize(LowlevelRenderer* pLLRenderer);
  void Shutdown();

  void Update(FSceneNode* Frame);
  void Render(FSceneNode* Frame);

  void OnLevelChange();

  void AddFrameLight(AActor* pLight, FVector* pLocation = nullptr);
  bool HasFrameLight(AActor* pLight);
protected:
  void CacheLights();

  static void effectNOP(uint32_t pLightIndex, AActor* pLight, D3DLIGHT9& outLight) {};
  static void effectDisco(uint32_t pLightIndex, AActor* pLight, D3DLIGHT9& outLight);

  bool CalculateLightInfo(AActor* pActor, LightInfo& pmInfo);
protected:
  enum class ReservedSlots
  {
    jcDentonLight = 0,
    discoLightStart = 1,
    discoLightEnd = discoLightStart + 16,
    MAX_RESERVED
  };
private:
  LowlevelRenderer* m_LLRenderer = nullptr;
  std::unordered_map<ELightEffect, std::function<void(uint32_t pLightIndex, AActor* pLight, D3DLIGHT9& outLight)>> m_Effects;
  uint32_t m_LevelLightsLowerBound = 0xFFFFFFFFul;
  uint32_t m_LevelLightsUpperBound = 0x00000000ul;
  AAugmentation* m_lightAugmentation = nullptr;
  uint32_t m_FreeFrameLightIndex = 0;
  uint32_t m_FreeLevelLightsExtraIndex = 0;
  std::vector<LightInfo> m_LevelLights;
  std::vector<LightInfo> m_FrameLights;
  std::vector<LightInfo> m_ExpiringFrameLights;
  std::vector<LightInfo> m_ExpiredFrameLights;
};