#pragma once

#include <string>
#include <filesystem>

namespace Utils
{
  extern std::wstring GetProcessFolder();
  extern std::string ConvertWcToUtf8(const std::wstring& pString);
  extern std::wstring ConvertUtf8ToWc(const std::string& pString);
  extern uint32_t HashWc(const wchar_t* pString);
  extern void PushOriginalFileManager();
  extern void PopOriginalFileManager();
  extern void Screenshot(HWND pHwnd, std::filesystem::path pFilePath);
  template <typename TDeduced1, typename TDeduced2>
  class ScopedCall
  {
  public:
    ScopedCall() = delete;
    ScopedCall(ScopedCall&) = delete;
    ScopedCall(ScopedCall&&) = delete;

    ScopedCall(const TDeduced1& pOnEnterFunction, const TDeduced2& pOnExitFunction) : m_OnEnter(pOnEnterFunction), m_OnExit(pOnExitFunction) { m_OnEnter(); }
    ~ScopedCall() { m_OnExit(); }
  private:
    TDeduced1 m_OnEnter;
    TDeduced2 m_OnExit;
  };
}