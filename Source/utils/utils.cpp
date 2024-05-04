#include "DeusExEchelonRenderer_PCH.h"
#pragma hdrstop
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <FFileManagerWindows.h>
#include <codecvt> // codecvt_utf8
#include <locale>  // wstring_convert
#include <filesystem>
#include "utils.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring Utils::GetProcessFolder()
{
  wchar_t buffer[2048]{ 0 };
  if (GetModuleFileNameW(nullptr, &buffer[0], std::size(buffer)) > 0)
  {
    std::filesystem::path p(buffer);
    if (p.has_parent_path())
    {
      return (p.parent_path() / std::filesystem::path{}).wstring();
    }
  }

  return L"." + std::filesystem::path::preferred_separator;
}

std::string Utils::ConvertWcToUtf8(const std::wstring& pString)
{
  return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(pString);
}

std::wstring Utils::ConvertUtf8ToWc(const std::string& pString)
{
  return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(pString);
}

std::optional<decltype(GFileManager)> g_OriginalFileManager;
void Utils::PushOriginalFileManager()
{
  check(!g_OriginalFileManager);
  g_OriginalFileManager = GFileManager;
  GFileManager = new FFileManagerWindows();
}

void Utils::PopOriginalFileManager()
{
  check(g_OriginalFileManager);
  delete(GFileManager);
  GFileManager = g_OriginalFileManager.value();
  g_OriginalFileManager.reset();
}