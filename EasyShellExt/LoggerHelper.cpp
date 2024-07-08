#include "pch.h"
#include "LoggerHelper.h"
#include <string>
#include "trace.h"

namespace esx {
LoggerHelper::LoggerHelper(LOG_LEVEL level) :
    filename_(NULL),
    line_(0),
    level_(level) {
}

LoggerHelper::LoggerHelper(const char* filename, int line, LOG_LEVEL level) :
    filename_(filename),
    line_(line),
    level_(level) {
}

LoggerHelper::~LoggerHelper() {
    std::string str = ss_.str();
    if (filename_) {
        Trace::MsgA("[ESX] %s %d %d %s", filename_, line_, level_, str.c_str());
    }
    else {
        Trace::MsgA("[ESX] %d %d %s", line_, level_, str.c_str());
    }
}

LoggerHelper& LoggerHelper::operator<<(std::ostream& (*f)(std::ostream&)) {
    if (f == std::endl) {
        ss_ << '\n';
    }
    return *this;
}

}  // namespace esx
