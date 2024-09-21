#pragma once

class ConfigManager
{
public:
  void LoadConfig();
  void SaveConfig();
public:
  std::vector<std::wstring>& GetHijackedTextureNames() { return m_HijackedTextureNames; }
  bool GetRenderPlayerBody() const { return m_RenderPlayerBody; }
  bool GetRenderSkybox() const { return m_RenderSkybox; }
  bool GetAggressiveMouseFix() const { return m_AggressiveMouseFix; }
private:
  std::vector<std::wstring> m_HijackedTextureNames;
  UBOOL m_RenderPlayerBody = true;
  UBOOL m_RenderSkybox = true;
  UBOOL m_AggressiveMouseFix = true;
};

extern ConfigManager g_ConfigManager;