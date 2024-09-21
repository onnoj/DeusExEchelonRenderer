#include <string>
#include <filesystem>

namespace Utils
{
  extern std::wstring GetProcessFolder();
  extern std::string ConvertWcToUtf8(const std::wstring& pString);
  extern std::wstring ConvertUtf8ToWc(const std::string& pString);
  extern std::wstring ToLower(const std::wstring& pStr);
  extern uint32_t HashWc(const wchar_t* pString);
  extern void PushOriginalFileManager();
  extern void PopOriginalFileManager();
  extern void Screenshot(HWND pHwnd, std::filesystem::path pFilePath);
  extern void ShowMessageBox(const TCHAR* pTitle, const TCHAR* pBody);

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

  template <typename T>
  T GetUEProperty(const TCHAR* pName, UObject* pObject)
  {
    auto propertyName = FName(pName, FNAME_Find);
    for (TFieldIterator<UField> it(pObject->GetClass()); it; ++it)
    {
      if (it->GetFName() == propertyName)
      {
        UProperty* prop = Cast<UProperty>(*it);
        return *(reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(pObject) + prop->Offset));
      }
    }

    return {0};
  }
}