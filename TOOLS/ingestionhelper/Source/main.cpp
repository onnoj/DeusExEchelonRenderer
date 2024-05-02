#include "IngestionHelper_PCH.h"
#pragma hdrstop

#include "main.h"
#include "ingestionhelper.h"
#include "utils.h"

/*
* --jsondump "F:\Games\Steam\steamapps\common\Deus Ex\System\textureDump.json" -i "D:\development\misc\DeusExRemix\DeusEx\imports\AllTextures" -o "D:\development\misc\DeusExRemix\DeusEx\imports\AllTextures_Categorized" -m copy --filter "path=.*glass.*" --filter "package=.*unatco.*"
*/

int main(int argc, char** argv)
{
  CLI::App app{"Ingestion Helper"};
  argv = app.ensure_utf8(argv);

  Options options{};
  {
    std::vector<std::string> jsonFilters;
    std::vector<std::string> inputFilters;
    std::map<std::string, Options::OperationMode> operationModeMap{{"copy", Options::OperationMode::copy}, {"unknown", Options::OperationMode::unknown}};

    app.add_option("-d,--jsondump", options.textureDumpJSON, "Path to the texture JSON dump")->required(false);
    app.add_option("-i,--input", options.inputFolderPath, "Path to the folder containing all the .dds files")->required(true);
    app.add_option("-o,--output", options.outputFolderPath, "Path to the folder that will contain all the output")->required(true);
    app.add_option("-m,--mode", options.operationMode, "Sets the operation to perform on the files. Currently supported: copy")->required(true)->transform(CLI::CheckedTransformer(operationModeMap, CLI::ignore_case));
    app.add_option("--inputfilenameformat", options.inputFilenameFormat, "The scnFormat string to try parse the ID from the filename, by default: \"" + options.inputFilenameFormat + "\"")->required(false);
    app.add_option("--outputformat", options.outputFormat, "The format for the output file operation, fields are sourced directly from the dump .json, can contain path seperators characters (\\). For example, by default: \"" + options.outputFormat +"\"")->required(false);
    app.add_option("--jsonfilter", jsonFilters, "Match against specific key/value pairs in the JSON. ie, \"path=.*Glass.*\"")->required(false);
    app.add_option("--inputfilter", inputFilters, "Match against specific files, ie, \".*_diffuse.png\"")->required(false);
    CLI11_PARSE(app, argc, argv);

    for (auto& s : jsonFilters)
    {
      auto p = Utils::StringSplit(s, "=");
      try
      {
        std::regex r(p.second, std::regex::icase);
        options.jsonFilterMap.insert(std::make_pair(p.first, std::move(r)));
      }
      catch (const std::regex_error& re)
      {
        std::cerr << "Invalid regex string: " << p.second << std::endl;
        return ERROR_INVALID_PARAMETER;
      }
    }

    for (auto& s : inputFilters)
    {
      try
      {
        std::regex r(s, std::regex::icase);
        options.inputFileFilters.push_back(std::move(r));
      }
      catch (const std::regex_error& re)
      {
        std::cerr << "Invalid regex string: " << s << std::endl;
        return ERROR_INVALID_PARAMETER;
      }
    }
  }

  std::ifstream jsonDump(options.textureDumpJSON);
  if (!jsonDump.is_open())
  {
    std::cerr << "JSON dump was not found" << std::endl;
    return ERROR_FILE_NOT_FOUND;
  }

  if (!std::filesystem::exists(options.inputFolderPath))
  {
    std::cerr << "Input folder does not exist" << std::endl;
    return ERROR_FILE_NOT_FOUND;
  }

  if (!std::filesystem::exists(options.outputFolderPath))
  {
    if (!std::filesystem::create_directories(options.outputFolderPath))
    {
      std::cerr << "Output folder did not exist and could not be created." << std::endl;
      return ERROR_FILE_NOT_FOUND;
    }
  }

  if (options.operationMode != Options::OperationMode::copy)
  {
    std::cerr << "Unsupported operation mode selected" << std::endl;
    return ERROR_INVALID_PARAMETER;
  }

  std::string jsonBlob((std::istreambuf_iterator<char>(jsonDump)),
    (std::istreambuf_iterator<char>()));
  jsonDump.close();

  using json = nlohmann::json;
  const json jsonStructure = json::parse(jsonBlob);
  if (!jsonStructure.contains("textures"))
  {
    std::cerr << "The json dump did not contain the expected array of textures." << std::endl;
    return -1;
  }

  IngestionHelper helper(jsonStructure, options);
  helper.Work();

  return 0;
}