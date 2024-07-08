#ifndef SHELL_EXT_TRACE_H__
#define SHELL_EXT_TRACE_H__
#pragma once

namespace esx {
class Trace {
   public:
    static void MsgA(const char* lpFormat, ...) noexcept;
};
}  // namespace esx

#endif  // !SHELL_EXT_TRACE_H__