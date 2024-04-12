#include "IngestionHelper_PCH.h"
#pragma hdrstop

#include "ingestionhelper.h"
using json = nlohmann::json;

void IngestionHelper::loadData()
{
  const json textureArray = m_Root["textures"];
  m_TextureDataArray.reserve(textureArray.size());
  for (const auto& textureJsonObj : textureArray)
  {
    TextureData textureData;
    textureData.m_CacheId = textureJsonObj["cacheid"];
    for (auto f : textureJsonObj["flags"])
    {
      textureData.m_Flags.push_back(f);
    }
    textureData.m_Index = textureJsonObj["index"];
    textureData.m_Name = textureJsonObj["name"];
    textureData.m_Package = textureJsonObj["package"];
    textureData.m_Path = textureJsonObj["path"];
    textureData.m_RealTime = textureJsonObj["realtime"];
    textureData.m_RemixHash = textureJsonObj["remixhash"];
    textureData.m_RemixHashHex = textureJsonObj["remixhash-hex"];
    m_TextureDataArray.push_back(std::move(textureData));
  }

  for (const auto& data : m_TextureDataArray)
  {
    m_HashToDataMap.insert(std::make_pair(data.m_RemixHash, &data));
  }
}

void IngestionHelper::processFiles()
{
  auto convertWStringToUTF8 = [](const wchar_t* pWString){
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return utf8_conv.to_bytes(pWString);
  };
  auto sanitizeName = [](std::string pName)
  {
    constexpr std::pair<const char*,const char*> strings[] = {
      {"-", "_"}, 
      {"/", "_"},
      {"<", "_"}, 
      {">", "_"},
      {":", "_"}, 
      {"\"", "_"},
      {"\\", "_"}, 
      {"|", "_"},
      {"?", "_"}, 
      {"*", "_"},
      {" ", "_"},
      {"\t", "_"},
    };
    for (auto pair : strings)
    {
      size_t pos = 0;
      while ((pos = pName.find(pair.first, pos)) != std::string::npos) {
        pName.replace(pos, strlen(pair.first), pair.second);
        pos += strlen(pair.second);
      }
    }
    return pName;
  };

  for (auto const& dir_entry : std::filesystem::directory_iterator{ m_InputFolder })
  {
    std::string pathUtf8 = convertWStringToUTF8(dir_entry.path().c_str());
    std::string filenameUtf8 = convertWStringToUTF8(dir_entry.path().filename().c_str());
    uint64_t fileHash = 0;

    auto res = scn::scan<uint64_t>(filenameUtf8, m_FilenameFormat);
    if (!res)
    {
      std::cout << dir_entry.path() << '\n';
    }
    fileHash = std::get<0>(res->values());
    if (auto it = m_HashToDataMap.find(fileHash); it != m_HashToDataMap.end())
    {
      auto data = it->second;
      std::string package = sanitizeName(data->m_Package);
      std::string name = sanitizeName(data->m_Name);
      std::string textureFolder = name + " - " + sanitizeName(data->m_RemixHashHex);

      std::filesystem::path textureOutputFolder = m_OutputFolder / std::filesystem::path(package) / std::filesystem::path(textureFolder);
      if (!std::filesystem::exists(textureOutputFolder))
      {
        if (!std::filesystem::create_directories(textureOutputFolder))
        {
          int x = 1;
        }
      }
    }
  }
}

void IngestionHelper::Work()
{
  loadData();
  processFiles();
}
