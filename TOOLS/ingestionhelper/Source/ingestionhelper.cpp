#include "IngestionHelper_PCH.h"
#pragma hdrstop

#include "ingestionhelper.h"
#include "utils.h"

using json = nlohmann::json;

void IngestionHelper::loadData()
{
  const json textureArray = m_Root["textures"];
  m_TextureDataArray.reserve(textureArray.size());
  for (const auto& textureJsonObj : textureArray)
  {
    TextureData textureData;
    textureData.m_JSONItem = textureJsonObj;
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
      {".", "_"},
      {"\t", "_"},
    };
    for (auto pair : strings)
    {
      Utils::StringReplaceAll(pName, pair.first, pair.second);
    }
    return pName;
  };

  for (auto const& inputFile : std::filesystem::directory_iterator{ m_Options.inputFolderPath })
  {
    std::string pathUtf8 = convertWStringToUTF8(inputFile.path().c_str());
    std::string filenameUtf8 = convertWStringToUTF8(inputFile.path().filename().c_str());
    uint64_t fileHash = 0;

    if (!m_Options.inputFileFilters.empty())
    {
      bool hasMatch = false;
      for (auto& r : m_Options.inputFileFilters)
      {
        hasMatch |= (hasMatch || (std::regex_match(filenameUtf8, r)));
      }
      if (!hasMatch)
      {
        //skip file
        continue;
      }
    }


    auto res = scn::scan<uint64_t, std::wstring>(filenameUtf8, m_Options.inputFilenameFormat);
    if (!res)
    {
      std::cout << inputFile.path() << '\n';
      return;
    }
    fileHash = std::get<0>(res->values());

    const bool hasJsonFilters = !m_Options.jsonFilterMap.empty();
    if (auto it = m_HashToDataMap.find(fileHash); it != m_HashToDataMap.end())
    {
      auto data = it->second;

      bool hasMatch = false;
      std::string fileNameCandidate = m_Options.outputFormat;
      for (auto& i : data->m_JSONItem.items())
      {
        std::string variable = "${";
        variable += i.key();
        variable += "}";

        std::string replacementValue = i.value().dump();
        if (Utils::StringStartsWith(replacementValue, "\"") && Utils::StringEndsWith(replacementValue, "\""))
        {
          replacementValue = replacementValue.substr(1);
          replacementValue.pop_back();
        }

        if (auto it = m_Options.jsonFilterMap.find(i.key()); it != m_Options.jsonFilterMap.end())
        {
          const auto& regex = it->second;
          hasMatch |= std::regex_match(replacementValue, regex);
        }
        
        replacementValue = sanitizeName(replacementValue);
        Utils::StringReplaceAll(fileNameCandidate, variable.c_str(), replacementValue.c_str());
      }
      Utils::StringReplaceAll(fileNameCandidate, "${filename}", filenameUtf8.c_str());
      

      if (hasJsonFilters && !hasMatch)
      { //Skip entry, we have filters and nothing matched...
        continue;
      }

      std::filesystem::path textureOutputFile = m_Options.outputFolderPath / fileNameCandidate;
      std::filesystem::path textureOutputFolder = textureOutputFile.parent_path();

      if (!std::filesystem::exists(textureOutputFolder))
      {
        if (!std::filesystem::create_directories(textureOutputFolder))
        {
          std::cerr << "Unable to create path: " << textureOutputFolder.string() << std::endl;
          return;
        }
      }

      switch (m_Options.operationMode)
      {
        case Options::OperationMode::copy:
        {
          std::filesystem::copy_file(inputFile, textureOutputFile, std::filesystem::copy_options::overwrite_existing);
        }; break;
        default: 
          assert(false/*unsupported operation*/);
      }

      int x = 1;
    }
  }
}

void IngestionHelper::Work()
{
  loadData();
  processFiles();
}
