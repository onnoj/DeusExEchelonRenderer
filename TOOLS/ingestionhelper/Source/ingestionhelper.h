#pragma once
using json = nlohmann::json;

struct Options
{
  std::string textureDumpJSON = "textureDump.json";
  std::filesystem::path inputFolderPath = "";
  std::filesystem::path outputFolderPath = "";
  std::string inputFilenameFormat = "{:X}";
  std::string outputFormat = "${package}\\${remixhash-hex}.png";
  std::vector<std::regex> inputFileFilters{};
  std::map<std::string, std::regex> jsonFilterMap{};
  enum class OperationMode
  {
    unknown,
    copy,
  } operationMode = OperationMode::unknown;
};

struct TextureData
{
  json m_JSONItem;
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
  IngestionHelper(const json& pJSONDumpStruct, const Options& options)
    : m_Root(pJSONDumpStruct),
      m_Options(options){};
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
  Options m_Options;
  std::unordered_map<uint64_t, const TextureData*> m_HashToDataMap;
  std::vector<TextureData> m_TextureDataArray;
};

