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

    auto rtxConfFile = modulePath.replace_filename("rtx.conf");
    if (std::filesystem::exists(rtxConfFile))
    {
      std::wifstream rtxConfig(rtxConfFile.native());
      if (!rtxConfig.is_open())
      {
        return;
      }

      bool foundMaxLights = false;
      auto wholeFile = std::wstring(std::istreambuf_iterator<wchar_t>(rtxConfig), std::istreambuf_iterator<wchar_t>());
      rtxConfig.seekg(std::ios_base::beg);

      std::wstring line;
      std::list<std::wstring> lines;
      while (std::getline(rtxConfig, line, L'\n'))
      {
        lines.push_back(line);
        std::transform(line.begin(), line.end(), line.begin(),[](unsigned char c){ return std::tolower(c); });
        if (line.find(L"d3d9.maxenabledlights") != -1)
        {
          foundMaxLights = true;
        }
      }
      rtxConfig.close();

      if (!foundMaxLights)
      {
        lines.push_back(L"d3d9.maxEnabledLights = 65000");
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

