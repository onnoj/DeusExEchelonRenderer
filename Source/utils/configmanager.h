#pragma once

class ConfigManager
{
public:
  void LoadConfig();
  void SaveConfig();
public:
  std::vector<std::string>& GetHijackedTextureNames() { return m_HijackedTextureNames; }
  bool GetRenderBody() const { return m_RenderBody; }
private:
  std::vector<std::string> m_HijackedTextureNames;
  UBOOL m_RenderBody = false;
};

extern ConfigManager g_ConfigManager;