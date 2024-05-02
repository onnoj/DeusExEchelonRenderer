#include "IngestionHelper_PCH.h"
#pragma hdrstop

#include "utils.h"

bool Utils::StringStartsWith(const std::string& pString, const char* pSearchFor)
{
  return (pString.find_first_of(pSearchFor) == 0);
}
bool Utils::StringEndsWith(const std::string& pString, const char* pSearchFor)
{
  auto endOfString = pString.length() - strlen(pSearchFor);
  return ((pString.find_last_of(pSearchFor) == endOfString));
}

void Utils::StringReplaceAll(std::string& pmString, const char* pSearchFor, const char* pReplaceWith)
{
  size_t pos = 0;
  while ((pos = pmString.find(pSearchFor, pos)) != std::string::npos) {
    pmString.replace(pos, strlen(pSearchFor), pReplaceWith);
    pos += strlen(pReplaceWith);
  }
}

std::pair<std::string, std::string> Utils::StringSplit(const std::string& pString, const std::string& pDelimeter)
{
  std::pair<std::string, std::string> result;

  size_t pos = pString.find(pDelimeter);

  if (pos != std::string::npos) { 
    result.first = pString.substr(0, pos);
    result.second = pString.substr(pos + pDelimeter.length());
  } else {
    result.first = pString;
    result.second = "";
  }

  return result;
}
