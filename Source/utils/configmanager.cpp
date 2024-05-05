#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop

#include "configmanager.h"

ConfigManager g_ConfigManager;

void ConfigManager::LoadConfig()
{
  Utils::ScopedCall scopedFileManager {
    [&]() { Utils::PushOriginalFileManager(); },
    [&]() { Utils::PopOriginalFileManager(); }
  };

  //Load the hijacked texture names. When loading textures, the renderer will match against this
  //array and instead generate textures with a unique rtx hashes for each texture+primitiveflag permutation.
  //The array is read greedily, we will add each entry to an array until either the key does not exist or
  //the entry is empty. When saving, we always generate an empty entry so that there is a default entry.
  {
    wchar_t buffer[32]{0};
    uint32_t index = 0;
    FString str;
    for(auto index = 0; ; index++)
    {
      _itow_s(index, buffer, 10);

      std::wstring key = L"HijackTexture[";
      key += buffer;
      key += L"]";

      if (!GConfig->GetString(g_RendererName L".TextureManager", key.c_str(), str, g_ConfigFilename.c_str()))
      {
        break;
      }

      auto textureId = Utils::ConvertWcToUtf8(*str);
      if (textureId.empty())
      {
        break;
      }

      m_HijackedTextureNames.push_back(textureId);
    }
  }

  //Misc properties
  GConfig->GetBool(g_RendererName L".Settings", L"RenderBody", m_RenderBody, g_ConfigFilename.c_str());
}


void ConfigManager::SaveConfig()
{
  Utils::ScopedCall scopedFileManager {
    [&]() { Utils::PushOriginalFileManager(); },
    [&]() { Utils::PopOriginalFileManager(); }
  };

  //Store the m_HijackedTextureNames array. See notes in the read section above.
  {
    wchar_t buffer[32]{ 0 };
    for (int i = 0; i < m_HijackedTextureNames.size(); i++)
    {
      _itow_s(i, buffer, 10);

      std::wstring key = L"HijackTexture[";
      key += buffer;
      key += L"]";

      std::wstring value = Utils::ConvertUtf8ToWc(m_HijackedTextureNames[i]);
      GConfig->SetString(g_RendererName L".TextureManager", key.c_str(), value.c_str(), g_ConfigFilename.c_str());
    }

    _itow_s(m_HijackedTextureNames.size(), buffer, 10);
    std::wstring key = L"HijackTexture[";
    key += buffer;
    key += L"]";
    GConfig->SetString(g_RendererName L".TextureManager", key.c_str(), L"", g_ConfigFilename.c_str());
  }

  //Misc properties
  GConfig->SetBool(g_RendererName L".Settings", L"RenderBody", m_RenderBody, g_ConfigFilename.c_str());

  //Store file
  GConfig->Flush(0, g_ConfigFilename.c_str());
}