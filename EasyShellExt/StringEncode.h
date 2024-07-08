#ifndef SHELL_EXT_STRING_ENCODE_HPP_
#define SHELL_EXT_STRING_ENCODE_HPP_
#pragma once

#include <string>

namespace esx {
class StringEncode {
   public:
    static std::string UnicodeToAnsi(const std::wstring& str, unsigned int code_page = 0) noexcept;
    static std::wstring AnsiToUnicode(const std::string& str, unsigned int code_page = 0) noexcept;
};
}  // namespace esx
#endif  // SHELL_EXT_STRING_ENCODE_HPP_
