#include "DeusExEchelonCore_PCH.h"
#pragma hdrstop

////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <FFileManagerWindows.h>
#include <codecvt> // codecvt_utf8
#include <locale>  // wstring_convert
#include <filesystem>
#include <optional>
#include "MurmurHash3.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "coreutils.h"
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

uint32_t Utils::HashWc(const wchar_t* pString)
{
  uint32_t ret = 0;
  ::MurmurHash3_x86_32(pString, lstrlenW(pString), 0, &ret);
  return ret;
}

std::string Utils::ConvertWcToUtf8(const std::wstring& pString)
{
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.to_bytes(pString);
}

std::wstring Utils::ConvertUtf8ToWc(const std::string& pString)
{
  static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.from_bytes(pString);
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

void Utils::Screenshot(HWND pHwnd, std::filesystem::path pFilePath)
{
  HDC hdcWindow = GetDC(pHwnd);
  HDC hdcMemDC = CreateCompatibleDC(hdcWindow);

  if (!hdcMemDC) {
    ReleaseDC(pHwnd, hdcWindow);
    throw std::runtime_error("CreateCompatibleDC failed");
  }

  RECT rcClient;
  GetClientRect(pHwnd, &rcClient);

  HBITMAP hbmWindow = CreateCompatibleBitmap(hdcWindow, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

  if (!hbmWindow) {
    DeleteDC(hdcMemDC);
    ReleaseDC(pHwnd, hdcWindow);
    throw std::runtime_error("CreateCompatibleBitmap failed");
  }

  SelectObject(hdcMemDC, hbmWindow);

  if (!PrintWindow(pHwnd, hdcMemDC, PW_RENDERFULLCONTENT|PW_CLIENTONLY)) {
    DeleteObject(hbmWindow);
    DeleteDC(hdcMemDC);
    ReleaseDC(pHwnd, hdcWindow);
    throw std::runtime_error("PrintWindow failed");
  }

  BITMAP bmp;
  GetObject(hbmWindow, sizeof(BITMAP), &bmp);

  BITMAPFILEHEADER bmfHeader{0};
  BITMAPINFOHEADER bi{0};

  bi.biSize = sizeof(BITMAPINFOHEADER);
  bi.biWidth = bmp.bmWidth;
  bi.biHeight = bmp.bmHeight;
  bi.biPlanes = 1;
  bi.biBitCount = 32;
  bi.biCompression = BI_RGB;
  bi.biSizeImage = 0;
  bi.biXPelsPerMeter = 0;
  bi.biYPelsPerMeter = 0;
  bi.biClrUsed = 0;
  bi.biClrImportant = 0;

  DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;

  HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
  char* lpbitmap = (char*)GlobalLock(hDIB);

  GetDIBits(hdcWindow, hbmWindow, 0, (UINT)bmp.bmHeight, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

  HANDLE hFile = CreateFile(pFilePath.wstring().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  DWORD dwSizeOfDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  bmfHeader.bfSize = dwSizeOfDIB;
  bmfHeader.bfType = 0x4D42; // BM

  DWORD dwBytesWritten = 0;
  WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
  WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
  WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

  GlobalUnlock(hDIB);
  GlobalFree(hDIB);
  CloseHandle(hFile);

  DeleteObject(hbmWindow);
  DeleteDC(hdcMemDC);
  ReleaseDC(pHwnd, hdcWindow);
}