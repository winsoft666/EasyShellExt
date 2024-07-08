#ifndef SHELL_EXT_LOGGER_HELPER_H
#define SHELL_EXT_LOGGER_HELPER_H
#pragma once

#include <ostream>
#include <sstream>

namespace esx {
enum LOG_LEVEL {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
};

class LoggerHelper {
   public:
    LoggerHelper(LOG_LEVEL level);
    LoggerHelper(const char* filename, int line, LOG_LEVEL level);
    virtual ~LoggerHelper();

    template <class Any>
    LoggerHelper& operator<<(const Any& any) {
        ss_ << any;
        return (*this);
    }

    LoggerHelper& operator<<(std::ostream& (*f)(std::ostream&));

   private:
    LOG_LEVEL level_;
    const char* filename_;
    int line_;
    std::stringstream ss_;
};

#ifndef ESX_LOG
#define ESX_LOG(expr) (::esx::LoggerHelper(__FILE__, __LINE__, ::esx::LOG_LEVEL_##expr))
#endif

}  // namespace esx

#endif  //SHELL_EXT_LOGGER_HELPER_H
