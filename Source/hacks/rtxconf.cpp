#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

#include "misc.h"
#include "hacks.h"

void InstallRTXConfigPatches()
{
  wchar_t modulePath[1000];
  if (GetModuleFileNameW(NULL, &modulePath[0], std::size(modulePath)) > 0)
  {
    std::wstring modulePathString = &modulePath[0];
    std::filesystem::path modulePath(modulePathString);
    auto moduleDir = modulePath.parent_path();

    constexpr std::tuple<wchar_t*, wchar_t*> defaultConfigOptions[] = {
      {L"d3d9.maxEnabledLights", L"65000"},
      {L"rtx.suppressLightKeeping", L"True"},
    };
    bool hasOptions[std::size(defaultConfigOptions)]{false};

    auto rtxConfFile = modulePath.replace_filename("rtx.conf");
    if (std::filesystem::exists(rtxConfFile))
    {
      std::wifstream rtxConfig(rtxConfFile.native());
      if (!rtxConfig.is_open())
      {
        return;
      }

      auto wholeFile = std::wstring(std::istreambuf_iterator<wchar_t>(rtxConfig), std::istreambuf_iterator<wchar_t>());
      rtxConfig.seekg(std::ios_base::beg);

      std::wstring line;
      std::list<std::wstring> lines;
      while (std::getline(rtxConfig, line, L'\n'))
      {
        lines.push_back(line);
        for (int i = 0; i < std::size(defaultConfigOptions); i++)
        {
          std::wstring key = std::get<0>(defaultConfigOptions[i]);
          std::wstring lowercaseKey = key;

          if (line.find(lowercaseKey) != -1)
          {
            hasOptions[i] = true;
          }
        }
      }
      rtxConfig.close();

      bool shouldRewrite = false;
      for (int i = 0; i < std::size(defaultConfigOptions); i++)
      {
        if (!hasOptions[i])
        {
          std::wstring key = std::get<0>(defaultConfigOptions[i]);
          std::wstring value = std::get<1>(defaultConfigOptions[i]);
          lines.push_back(key + L" = " + value);
          shouldRewrite = true;
        }
      }

      if (shouldRewrite)
      {
        std::wofstream rtxConfig(rtxConfFile.native(), std::ios_base::out | std::ios_base::trunc);
        if (rtxConfig.is_open())
        {
          for (auto& l : lines)
          {
            while (std::remove_if(l.begin(), l.end(), [](wchar_t pCmp) { return (pCmp == '\r'); }) != l.end()) {};
            rtxConfig.write(l.c_str(), l.length());
            rtxConfig.write(L"\n", 1);
          }
        }
      }
    }
  }
}

void UninstallRTXConfigPatches()
{
}

