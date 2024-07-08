#include "pch.h"
#include "trace.h"
#include <memory>
#ifndef _INC_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // !WIN32_LEAN_AND_MEAN
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif  // !_WINSOCKAPI_
#include <Windows.h>
#endif
#include <strsafe.h>
#include <string>
#include "Utils.h"

namespace esx {
void Trace::MsgA(const char* lpFormat, ...) noexcept {
    std::string output;
    va_list args;
    va_start(args, lpFormat);
    const bool ret = StringPrintfV(lpFormat, args, output);
    va_end(args);

    if (ret) {
        OutputDebugStringA(output.c_str());
    }
}
}  // namespace esx