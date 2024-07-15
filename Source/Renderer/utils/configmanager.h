#pragma once

class ConfigManager
{
public:
  void LoadConfig();
  void SaveConfig();
public:
  std::vector<std::string>& GetHijackedTextureNames() { return m_HijackedTextureNames; }
  bool GetRenderPlayerBody() const { return m_RenderPlayerBody; }
private:
  std::vector<std::string> m_HijackedTextureNames;
  UBOOL m_RenderPlayerBody = false;
};

extern ConfigManager g_ConfigManager;