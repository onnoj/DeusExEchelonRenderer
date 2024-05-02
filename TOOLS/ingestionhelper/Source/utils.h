#pragma once

struct Utils
{
  static void StringReplaceAll(std::string& pmString, const char* pSearchFor, const char* pReplaceWith);
  static bool StringStartsWith(const std::string& pString, const char* pSearchFor);
  static bool StringEndsWith(const std::string& pString, const char* pSearchFor);
  static std::pair<std::string, std::string> StringSplit(const std::string& pString, const std::string& pDelimeter);
};