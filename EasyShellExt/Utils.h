#ifndef SHELL_EXT_UTILS_H_
#define SHELL_EXT_UTILS_H_

#pragma once
#include <string>
#include <cctype>
#include <cstdarg>

namespace esx {
std::string GuidToString(GUID guid);
std::string GuidToInterfaceName(GUID guid);
bool StringPrintfV(const char* format, va_list argList, std::string& output) noexcept;
std::string StringPrintf(const char* format, ...) noexcept;
std::string ToHexString(void* value);
std::string GetQueryContextMenuFlags(UINT flags);
std::string GetGetCommandStringFlags(UINT flags);
std::string GetMenuTree(HMENU hMenu, int indent);
std::string GetProcessContextDesc();
std::wstring CLSIDToString(CLSID c);
}  // namespace esx

#endif  // !SHELL_EXT_UTILS_H_
