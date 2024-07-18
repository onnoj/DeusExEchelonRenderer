#pragma once

class ConfigManager
{
public:
  void LoadConfig();
  void SaveConfig();
public:
  std::vector<std::wstring>& GetHijackedTextureNames() { return m_HijackedTextureNames; }
  bool GetRenderPlayerBody() const { return m_RenderPlayerBody; }
private:
  std::vector<std::wstring> m_HijackedTextureNames;
  UBOOL m_RenderPlayerBody = true;
};

extern ConfigManager g_ConfigManager;