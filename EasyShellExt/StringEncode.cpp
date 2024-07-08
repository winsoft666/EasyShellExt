#include "pch.h"
#include "StringEncode.h"

#include <codecvt>
#include <locale>
#ifndef _INC_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif  // !_WINSOCKAPI_
#include <Windows.h>
#endif  // !_INC_WINDOWS

#pragma warning(disable : 4309)

namespace esx {
std::string StringEncode::UnicodeToAnsi(const std::wstring& str, unsigned int code_page) noexcept {
    std::string strRes;
    int iSize = ::WideCharToMultiByte(code_page, 0, str.c_str(), -1, NULL, 0, NULL, NULL);

    if (iSize == 0)
        return strRes;

    char* szBuf = new (std::nothrow) char[iSize];

    if (!szBuf)
        return strRes;

    memset(szBuf, 0, iSize);

    ::WideCharToMultiByte(code_page, 0, str.c_str(), -1, szBuf, iSize, NULL, NULL);

    strRes = szBuf;
    delete[] szBuf;

    return strRes;
}

std::wstring StringEncode::AnsiToUnicode(const std::string& str, unsigned int code_page) noexcept {
    std::wstring strRes;

    int iSize = ::MultiByteToWideChar(code_page, 0, str.c_str(), -1, NULL, 0);

    if (iSize == 0)
        return strRes;

    wchar_t* szBuf = new (std::nothrow) wchar_t[iSize];

    if (!szBuf)
        return strRes;

    memset(szBuf, 0, iSize * sizeof(wchar_t));

    ::MultiByteToWideChar(code_page, 0, str.c_str(), -1, szBuf, iSize);

    strRes = szBuf;
    delete[] szBuf;

    return strRes;
}
}  // namespace esx