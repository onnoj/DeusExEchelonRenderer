#include "IngestionHelper_PCH.h"
#pragma hdrstop

#include "main.h"
#include "ingestionhelper.h"

int main(int argc, char** argv)
{
  CLI::App app{"Ingestion Helper"};
  argv = app.ensure_utf8(argv);

  std::string textureDumpJSON = "textureDump.json";
  std::string inputFolderPath = "";
  std::string outputFolderPath = "";
  std::string filenameFormat = "{:X}";
  {
    app.add_option("-d,--jsondump", textureDumpJSON, "Path to the texture JSON dump")->required(false);
    app.add_option("-i,--input", inputFolderPath, "Path to the folder containing all the .dds files")->required(true);
    app.add_option("-o,--output", outputFolderPath, "Path to the folder that will contain all the output")->required(true);
    app.add_option("--filenameformat", filenameFormat, "The scn format string to parse the ID from the filename, by default: " + filenameFormat)->required(false);
    CLI11_PARSE(app, argc, argv);
  }

  std::ifstream jsonDump(textureDumpJSON);
  if (!jsonDump.is_open())
  {
    std::cerr << "JSON dump was not found" << std::endl;
    return ERROR_FILE_NOT_FOUND;
  }

  std::filesystem::path inputFolder(inputFolderPath);
  if (!std::filesystem::exists(inputFolder))
  {
    std::cerr << "Input folder does not exist" << std::endl;
    return ERROR_FILE_NOT_FOUND;
  }

  std::filesystem::path outputFolder(outputFolderPath);
  if (!std::filesystem::exists(inputFolder))
  {
    if (!std::filesystem::create_directories(outputFolder))
    {
      std::cerr << "Output folder did not exist and could not be created." << std::endl;
      return ERROR_FILE_NOT_FOUND;
    }
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

  IngestionHelper helper(jsonStructure, filenameFormat, inputFolder, outputFolder);
  helper.Work();

  return 0;
}