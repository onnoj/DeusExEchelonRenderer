#pragma once

class ConfigManager
{
public:
  void LoadConfig();
  void SaveConfig();
public:
  std::vector<std::string>& GetHijackedTextureNames() { return m_HijackedTextureNames; }
private:
  std::vector<std::string> m_HijackedTextureNames;
};

extern ConfigManager g_ConfigManager;