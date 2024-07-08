#include "pch.h"
#include "WinRegistry.h"
#include <process.h>
#include <strsafe.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)     \
    do {                    \
        if ((p)) {          \
            (p)->Release(); \
            (p) = NULL;     \
        }                   \
    } while (false)
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) \
    do {                     \
        if ((p) != NULL) {   \
            delete[] (p);    \
            (p) = NULL;      \
        }                    \
    } while (false)
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)     \
    do {                   \
        if ((p) != NULL) { \
            delete (p);    \
            (p) = NULL;    \
        }                  \
    } while (false)
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(p)       \
    do {                   \
        if ((p) != NULL) { \
            free(p);       \
            (p) = NULL;    \
        }                  \
    } while (false)
#endif

namespace esx {
WinRegistry::WinRegistry(HKEY hkeyRoot, const std::wstring& subKey) noexcept :
    m_hkeyRoot(hkeyRoot),
    m_hkey(NULL),
    m_hChangeEvent(NULL),
    m_hNotifyThr(NULL),
    m_bWatchSubtree(false),
    m_dwChangeFilter(0),
    m_dwSamDesired(0),
    m_strSubKey(subKey) {}

WinRegistry::~WinRegistry() noexcept {
    close();

    if (m_hChangeEvent)
        CloseHandle(m_hChangeEvent);
}

HRESULT WinRegistry::open(REGSAM samDesired, bool bCreate) noexcept {
    LSTATUS dwResult;
    close();

    m_dwSamDesired = samDesired;
    if (bCreate) {
        DWORD dwDisposition;
        dwResult = RegCreateKeyExW(m_hkeyRoot, m_strSubKey.c_str(), 0, NULL, 0, samDesired, NULL,
                                   &m_hkey, &dwDisposition);
    }
    else {
        dwResult = RegOpenKeyExW(m_hkeyRoot, m_strSubKey.c_str(), 0, samDesired, &m_hkey);
    }

    return HRESULT_FROM_WIN32(dwResult);
}

bool WinRegistry::isOpen() const noexcept {
    return NULL != m_hkey;
}

HKEY WinRegistry::getHandle() const noexcept {
    return m_hkey;
}

void WinRegistry::close() noexcept {
    if (NULL != m_hkey) {
        HKEY hkeyTemp = m_hkey;
        m_hkey = NULL;
        RegCloseKey(hkeyTemp);
    }

    if (m_hNotifyThr) {
        WaitForSingleObject(m_hNotifyThr, INFINITE);
    }

    if (m_hNotifyThr) {
        CloseHandle(m_hNotifyThr);
        m_hNotifyThr = NULL;
    }

    m_dwSamDesired = 0;
}

bool WinRegistry::watchForChange(DWORD dwChangeFilter, bool bWatchSubtree) noexcept {
    if (NULL != m_hChangeEvent || NULL == m_hkey)
        return false;

    m_hChangeEvent = CreateEventW(NULL, FALSE, FALSE, NULL);

    if (NULL == m_hChangeEvent) {
        return false;
    }

    m_dwChangeFilter = dwChangeFilter;
    m_bWatchSubtree = bWatchSubtree;

    unsigned int uThreadId = 0;
    m_hNotifyThr = (HANDLE)_beginthreadex(NULL, 0, NotifyWaitThreadProc, this, 0, &uThreadId);

    return !!m_hNotifyThr;
}

HRESULT WinRegistry::waitForChange(DWORD dwChangeFilter, bool bWatchSubtree) noexcept {
    LSTATUS ls = RegNotifyChangeKeyValue(m_hkey, bWatchSubtree, dwChangeFilter, NULL, FALSE);
    return HRESULT_FROM_WIN32(ls);
}

HRESULT WinRegistry::getDWORDValue(LPCWSTR pszValueName, OUT DWORD& pdwDataOut) const noexcept {
    return getValue(pszValueName, REG_DWORD, (LPBYTE)(&pdwDataOut), sizeof(DWORD));
}

HRESULT WinRegistry::getBINARYValue(LPCWSTR pszValueName, LPBYTE pbDataOut, int cbDataOut) const noexcept {
    return getValue(pszValueName, REG_BINARY, pbDataOut, cbDataOut);
}

HRESULT WinRegistry::getSZValue(LPCWSTR pszValueName, OUT std::wstring& strValue) const noexcept {
    DWORD cb = 0;
    HRESULT hr = getValueBufferSize(pszValueName, cb);
    if (FAILED(hr)) {
        return hr;
    }

    if (cb == 0) {
        strValue = L"";
        return S_OK;
    }

    WCHAR* pTemp = (WCHAR*)malloc(cb + sizeof(WCHAR));
    if (!pTemp)
        return E_OUTOFMEMORY;

    memset(pTemp, 0, cb + sizeof(WCHAR));

    hr = getValue(pszValueName, REG_SZ, (LPBYTE)pTemp, cb);
    if (SUCCEEDED(hr))
        strValue = pTemp;

    SAFE_FREE(pTemp);

    return hr;
}

HRESULT WinRegistry::getExpandSZValue(LPCWSTR pszValueName,
                                      bool bRetrieveExpandedString,
                                      OUT std::wstring& strValue) const noexcept {
    WCHAR szBuf[MAX_PATH] = {0};
    DWORD dwSize = MAX_PATH * sizeof(WCHAR);
    DWORD dwFlags = RRF_RT_ANY | RRF_ZEROONFAILURE;
    if (!bRetrieveExpandedString) {
        dwFlags |= RRF_NOEXPAND;
        dwFlags |= RRF_RT_REG_EXPAND_SZ;
    }
    else {
        dwFlags |= RRF_RT_REG_SZ;
    }

    if (m_dwSamDesired & KEY_WOW64_64KEY)
        dwFlags |= RRF_SUBKEY_WOW6464KEY;
    else if (m_dwSamDesired & KEY_WOW64_32KEY)
        dwFlags |= RRF_SUBKEY_WOW6432KEY;
    else
        dwFlags |= RRF_SUBKEY_WOW6432KEY;

    LSTATUS status =
        RegGetValueW(m_hkeyRoot, m_strSubKey.c_str(), pszValueName, dwFlags, NULL, szBuf, &dwSize);
    if (status == ERROR_MORE_DATA) {
        WCHAR* pBuf = new (std::nothrow) WCHAR[dwSize / sizeof(WCHAR)];
        if (pBuf) {
            memset(pBuf, 0, dwSize);
            status =
                RegGetValueW(m_hkeyRoot, m_strSubKey.c_str(), pszValueName, dwFlags, NULL, pBuf, &dwSize);

            strValue = pBuf;
            SAFE_DELETE_ARRAY(pBuf);
        }
        else {
            status = ERROR_OUTOFMEMORY;
        }
    }
    else if (status == ERROR_SUCCESS) {
        strValue = szBuf;
    }

    return HRESULT_FROM_WIN32(status);
}

HRESULT WinRegistry::getMultiSZValue(LPCWSTR pszValueName, OUT std::vector<std::wstring>& vStrValues) const noexcept {
    DWORD cb = 0;
    HRESULT hr = getValueBufferSize(pszValueName, cb);

    if (FAILED(hr)) {
        return hr;
    }

    if (cb == 0) {
        vStrValues.clear();
        return S_OK;
    }

    WCHAR* pTemp = (WCHAR*)malloc(cb + sizeof(WCHAR));
    if (!pTemp)
        return E_OUTOFMEMORY;

    memset(pTemp, 0, cb + sizeof(WCHAR));
    WCHAR* pBegin = pTemp;

    hr = getValue(pszValueName, REG_MULTI_SZ, (LPBYTE)pTemp, cb);
    if (SUCCEEDED(hr)) {
        while (pTemp && TEXT('\0') != *pTemp) {
            vStrValues.push_back(std::wstring(pTemp));
            pTemp += lstrlenW(pTemp) + 1;
        }
    }

    SAFE_FREE(pBegin);

    return hr;
}

HRESULT WinRegistry::getValueBufferSize(LPCWSTR pszValueName, DWORD& dwSize) const noexcept {
    DWORD dwType;
    DWORD cbData = 0;
    LSTATUS ls = RegQueryValueExW(m_hkey, pszValueName, 0, &dwType, NULL, (LPDWORD)&cbData);
    if (ls == ERROR_SUCCESS) {
        dwSize = cbData;
    }
    return HRESULT_FROM_WIN32(ls);
}

HRESULT WinRegistry::setDWORDValue(LPCWSTR pszValueName, DWORD dwData) noexcept {
    return setValue(pszValueName, REG_DWORD, (const LPBYTE)&dwData, sizeof(dwData));
}

HRESULT WinRegistry::setBINARYValue(LPCWSTR pszValueName, const LPBYTE pbData, int cbData) noexcept {
    return setValue(pszValueName, REG_BINARY, pbData, cbData);
}

HRESULT WinRegistry::setSZValue(LPCWSTR pszValueName, const std::wstring& strData) noexcept {
    return setValue(pszValueName, REG_SZ, (const LPBYTE)strData.c_str(),
                    ((DWORD)strData.length() + 1) * sizeof(WCHAR));
}

HRESULT WinRegistry::setExpandSZValue(LPCWSTR pszValueName, const std::wstring& strData) noexcept {
    return setValue(pszValueName, REG_EXPAND_SZ, (const LPBYTE)strData.c_str(),
                    ((DWORD)strData.length() + 1) * sizeof(WCHAR));
}

HRESULT WinRegistry::setMultiSZValue(LPCWSTR pszValueName, const std::vector<std::wstring>& vStrValues) noexcept {
    WCHAR* ptrValues = createDoubleNulTermList(vStrValues);
    if (!ptrValues) {
        return E_OUTOFMEMORY;
    }

    size_t cch = 1;
    size_t n = vStrValues.size();

    for (size_t i = 0; i < n; i++)
        cch += vStrValues[i].length() + 1;

    HRESULT hr = setValue(pszValueName, REG_MULTI_SZ, (const LPBYTE)ptrValues, (DWORD)cch * sizeof(TCHAR));

    SAFE_DELETE_ARRAY(ptrValues);

    return hr;
}

HRESULT WinRegistry::getSubKeys(std::vector<std::wstring>& subKeys) noexcept {
    WCHAR achKey[256];               // buffer for subkey name
    DWORD cbName = 255;              // size of name string
    WCHAR achClass[MAX_PATH] = L"";  // buffer for class name
    DWORD cchClassName = MAX_PATH;   // size of class string
    DWORD cSubKeys = 0;              // number of subkeys
    DWORD cbMaxSubKey;               // longest subkey size
    DWORD cchMaxClass;               // longest class string
    DWORD cValues;                   // number of values for key
    DWORD cchMaxValue;               // longest value name
    DWORD cbMaxValueData;            // longest value data
    DWORD cbSecurityDescriptor;      // size of security descriptor
    FILETIME ftLastWriteTime;        // last write time

    subKeys.clear();

    LSTATUS ls = RegQueryInfoKeyW(m_hkey,                 // key handle
                                  achClass,               // buffer for class name
                                  &cchClassName,          // size of class string
                                  NULL,                   // reserved
                                  &cSubKeys,              // number of subkeys
                                  &cbMaxSubKey,           // longest subkey size
                                  &cchMaxClass,           // longest class string
                                  &cValues,               // number of values for this key
                                  &cchMaxValue,           // longest value name
                                  &cbMaxValueData,        // longest value data
                                  &cbSecurityDescriptor,  // security descriptor
                                  &ftLastWriteTime);      // last write time

    if (ls != ERROR_SUCCESS)
        return HRESULT_FROM_WIN32(ls);

    for (DWORD i = 0; i < cSubKeys; i++) {
        cbName = 255;
        ls = RegEnumKeyExW(m_hkey, i, achKey, &cbName, NULL, NULL, NULL, &ftLastWriteTime);
        if (ls != ERROR_SUCCESS) {
            break;
        }

        subKeys.push_back(achKey);
    }

    return HRESULT_FROM_WIN32(ls);
}

void WinRegistry::OnChange(HKEY hkey) noexcept {
    UNREFERENCED_PARAMETER(hkey);
    //
    // Default does nothing.
    //
}

HRESULT WinRegistry::getValue(LPCWSTR pszValueName, DWORD dwTypeExpected, LPBYTE pbData, DWORD cbData) const noexcept {
    DWORD dwType = 0;
    LSTATUS ls = RegQueryValueExW(m_hkey, pszValueName, 0, &dwType, pbData, (LPDWORD)&cbData);

    if (ERROR_SUCCESS == ls && dwType != dwTypeExpected)
        ls = ERROR_INVALID_DATATYPE;

    return HRESULT_FROM_WIN32(ls);
}

HRESULT WinRegistry::setValue(LPCWSTR pszValueName, DWORD dwValueType, const LPBYTE pbData, DWORD cbData) noexcept {
    LSTATUS ls = RegSetValueExW(m_hkey, pszValueName, 0, dwValueType, pbData, cbData);
    return HRESULT_FROM_WIN32(ls);
}

LPWSTR WinRegistry::createDoubleNulTermList(const std::vector<std::wstring>& vStrValues) const noexcept {
    size_t cEntries = vStrValues.size();
    size_t cch = 1;  // Account for 2nd null terminate.

    for (size_t i = 0; i < cEntries; i++)
        cch += vStrValues[i].length() + 1;

    LPWSTR pszBuf = new (std::nothrow) WCHAR[cch];
    if (!pszBuf)
        return NULL;

    LPWSTR pszWrite = pszBuf;

    for (size_t i = 0; i < cEntries; i++) {
        const std::wstring& s = vStrValues[i];
        StringCchCopyW(pszWrite, cch, s.c_str());
        pszWrite += s.length() + 1;
    }

    *pszWrite = L'\0';  // Double null terminate.
    return pszBuf;
}

unsigned int _stdcall WinRegistry::NotifyWaitThreadProc(LPVOID pvParam) {
    WinRegistry* pThis = (WinRegistry*)pvParam;

    while (NULL != pThis->m_hkey) {
        LONG lResult = RegNotifyChangeKeyValue(pThis->m_hkey, pThis->m_bWatchSubtree,
                                               pThis->m_dwChangeFilter, pThis->m_hChangeEvent, true);

        if (ERROR_SUCCESS != lResult) {
            return 0;
        }
        else {
            switch (WaitForSingleObject(pThis->m_hChangeEvent, INFINITE)) {
                case WAIT_OBJECT_0:
                    if (NULL != pThis->m_hkey) {
                        pThis->OnChange(pThis->m_hkey);
                    }

                    break;

                case WAIT_FAILED:
                    break;

                default:
                    break;
            }
        }
    }

    return 0;
}

LSTATUS WinRegistry::DeleteKey(HKEY hKey, LPCWSTR pszSubKey, LPCWSTR pszValName) noexcept {
    LSTATUS ls = ERROR_NOT_SUPPORTED;
    HKEY hSubKey = NULL;

    if (!hKey || !pszValName) {
        return ERROR_INVALID_PARAMETER;
    }

    if (pszSubKey) {
        ls = RegOpenKeyExW(hKey, pszSubKey, 0, KEY_READ | KEY_WRITE, &hSubKey);
        if (ls != ERROR_SUCCESS)
            return ls;
    }
    else {
        hSubKey = hKey;
    }

    if (hSubKey) {
        ls = RegDeleteValueW(hSubKey, pszValName);

        if (hSubKey != hKey) {
            if (hSubKey)
                RegCloseKey(hSubKey);
        }
    }

    return ls;
}

}  // namespace esx