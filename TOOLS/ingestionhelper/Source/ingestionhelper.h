#pragma once

struct TextureData
{
  uint32_t m_CacheId = 0;
  std::vector<std::string> m_Flags;
  uint32_t m_Index = 0;
  std::string m_Name;
  std::string m_Package;
  std::string m_Path;
  bool m_RealTime = false;
  uint64_t m_RemixHash = 0;
  std::string m_RemixHashHex;
};


class IngestionHelper
{
  using json = nlohmann::json;
public:
  IngestionHelper(const json& pJSONDumpStruct, const std::string pFilenameFormat, const std::filesystem::path& pInputFolder, const std::filesystem::path& pOutputFolder)
    : m_Root(pJSONDumpStruct),
      m_FilenameFormat(pFilenameFormat),
      m_InputFolder(pInputFolder),
      m_OutputFolder(pOutputFolder) {};
  IngestionHelper() = delete;
  virtual ~IngestionHelper() = default;
  IngestionHelper(const IngestionHelper&) = delete;
  IngestionHelper(const IngestionHelper&&) = delete;

  void Work();
protected:
  void loadData();
  void processFiles();
private:
  json m_Root;
  std::string m_FilenameFormat;
  std::filesystem::path m_InputFolder;
  std::filesystem::path m_OutputFolder;
  std::unordered_map<uint64_t, const TextureData*> m_HashToDataMap;
  std::vector<TextureData> m_TextureDataArray;
};