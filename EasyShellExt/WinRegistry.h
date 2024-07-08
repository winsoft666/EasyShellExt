#ifndef ASHE_WIN_REGISTRY_HPP_
#define ASHE_WIN_REGISTRY_HPP_
#pragma once

#ifndef _INC_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // !WIN32_LEAN_AND_MEAN
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif  // !_WINSOCKAPI_
#include <Windows.h>
#endif
#include <vector>
#include <string>

namespace esx {
class WinRegistry {
   public:
    // hkeyRoot can be one of :
    // HKEY_CLASSES_ROOT
    // HKEY_CURRENT_CONFIG
    // HKEY_CURRENT_USER
    // HKEY_LOCAL_MACHINE
    // HKEY_USERS
    //
    WinRegistry(HKEY hkeyRoot, const std::wstring& subKey) noexcept;

    ~WinRegistry() noexcept;

    // samDesired:
    // https://docs.microsoft.com/zh-cn/windows/desktop/SysInfo/registry-key-security-and-access-rights
    // samDesired can be one of:
    // KEY_ALL_ACCESS,
    // KEY_QUERY_VALUE,
    // KEY_READ,
    // KEY_WRITE,
    // KEY_WOW64_32KEY,
    // KEY_WOW64_64KEY
    // and so on.
    //
    HRESULT open(REGSAM samDesired, bool bCreate) noexcept;

    bool isOpen() const noexcept;

    HKEY getHandle() const noexcept;

    void close() noexcept;

    bool watchForChange(DWORD dwChangeFilter, bool bWatchSubtree) noexcept;

    HRESULT waitForChange(DWORD dwChangeFilter, bool bWatchSubtree) noexcept;

    HRESULT getDWORDValue(LPCWSTR pszValueName, OUT DWORD& pdwDataOut) const noexcept;

    HRESULT getBINARYValue(LPCWSTR pszValueName, LPBYTE pbDataOut, int cbDataOut) const noexcept;

    HRESULT getSZValue(LPCWSTR pszValueName, OUT std::wstring& strValue) const noexcept;

    HRESULT getExpandSZValue(LPCWSTR pszValueName,
                             bool bRetrieveExpandedString,
                             OUT std::wstring& strValue) const noexcept;

    HRESULT getMultiSZValue(LPCWSTR pszValueName, OUT std::vector<std::wstring>& vStrValues) const noexcept;

    HRESULT getValueBufferSize(LPCWSTR pszValueName, DWORD& dwSize) const noexcept;

    HRESULT setDWORDValue(LPCWSTR pszValueName, DWORD dwData) noexcept;

    HRESULT setBINARYValue(LPCWSTR pszValueName, const LPBYTE pbData, int cbData) noexcept;
    HRESULT setSZValue(LPCWSTR pszValueName, const std::wstring& strData) noexcept;

    HRESULT setExpandSZValue(LPCWSTR pszValueName, const std::wstring& strData) noexcept;

    HRESULT setMultiSZValue(LPCWSTR pszValueName, const std::vector<std::wstring>& vStrValues) noexcept;

    HRESULT getSubKeys(std::vector<std::wstring>& subKeys) noexcept;

    static LSTATUS DeleteKey(HKEY hKey, LPCWSTR pszSubKey, LPCWSTR pszValName) noexcept;

   protected:
    void OnChange(HKEY hkey) noexcept;

   private:
    HRESULT getValue(LPCWSTR pszValueName, DWORD dwTypeExpected, LPBYTE pbData, DWORD cbData) const noexcept;
    HRESULT setValue(LPCWSTR pszValueName, DWORD dwValueType, const LPBYTE pbData, DWORD cbData) noexcept;
    LPWSTR createDoubleNulTermList(const std::vector<std::wstring>& vStrValues) const noexcept;

    static unsigned int _stdcall NotifyWaitThreadProc(LPVOID pvParam);

   private:
    HKEY m_hkeyRoot;
    mutable HKEY m_hkey;
    HANDLE m_hChangeEvent;
    HANDLE m_hNotifyThr;
    DWORD m_dwSamDesired;
    DWORD m_dwChangeFilter;
    std::wstring m_strSubKey;
    bool m_bWatchSubtree;
};
}  // namespace esx
#endif  // !ASHE_WIN_REGISTRY_HPP_
