#ifndef __GALILEO_BASE_LOGGING_H__
#define __GALILEO_BASE_LOGGING_H__

#include "config.h"
#include "basetypes.h"

#include <string>

#ifdef NO_GALILEO_LOGGING
#include <sstream>
#endif

namespace galileo {

enum class LogSeverity {
  LOG_INFO,
  LOG_WARNING,
  LOG_ERROR,
  LOG_ERROR_REPORT,
  LOG_FATAL,
  LOG_0 = LOG_ERROR
};

class Logger {
public:
  Logger(LogSeverity severity, const char* file, int line)
    : severity_(severity)
    , file_(file)
    , line_(line) {
  }

  ~Logger();

  // not private so that the operator<< overloads can get to it
  void printf(const char* fmt, ...);

private:
  LogSeverity   severity_;
  const char   *file_;
  int           line_;
  std::string   message_;

  DISALLOW_COPY_AND_ASSIGN(Logger);
};

class LogWrapper {
public:
  LogWrapper(LogSeverity severity, const char* file, int line) :
    log(severity, file, line) {
  }

  operator Logger&() const { return log; }

private:
  mutable Logger log;

  DISALLOW_COPY_AND_ASSIGN(LogWrapper);
};

struct EmptyLog {
};

} // namespace galileo

galileo::Logger& operator<<(galileo::Logger& log, const char* s);
galileo::Logger& operator<<(galileo::Logger& log, const std::string& s);
galileo::Logger& operator<<(galileo::Logger& log, int i);
galileo::Logger& operator<<(galileo::Logger& log, void* p);

template<class T>
const galileo::EmptyLog& operator<<(const galileo::EmptyLog& log, const T&) {
  return log;
}

#ifdef NO_GALILEO_LOGGING
#define GALILEO_LOG(info) std::stringstream()
#define LOG_IF(info, condition) if (!(condition)) std::stringstream()
#else
#define GALILEO_LOG(info) galileo::LogWrapper(galileo::LogSeverity::LOG_ ## info, __FILE__, __LINE__)
#define LOG_IF(info, condition) \
  if (!(condition)) galileo::LogWrapper(galileo::LogSeverity::LOG_ ## info, __FILE__, __LINE__)
#endif // NO_GALILEO_LOGGING

#ifdef DEBUG
#define DLOG(info) GALILEO_LOG(info)
#define DLOG_IF(info) LOG_IF(info)
#define DCHECK(condition) CHECK(condition)
#else
#define DLOG(info) galileo::EmptyLog()
#define DLOG_IF(info, condition) galileo::EmptyLog()
#define DCHECK(condition) while (false && (condition)) galileo::EmptyLog()
#endif // DEBUG

#undef LOG_ASSERT
#define LOG_ASSERT(cond) CHECK(0)
#define DLOG_ASSERT(cond) DCHECK(0)

#define NOTREACHED() GALILEO_LOG(ERROR)
#define NOTIMPLEMENTED() GALILEO_LOG(ERROR)

#undef CHECK
#define CHECK(condition) LOG_IF(FATAL, condition)

#define DCHECK_EQ(v1, v2) DCHECK((v1) == (v2))
#define DCHECK_NE(v1, v2) DCHECK((v1) != (v2))
#define DCHECK_LE(v1, v2) DCHECK((v1) <= (v2))
#define DCHECK_LT(v1, v2) DCHECK((v1) < (v2))
#define DCHECK_GE(v1, v2) DCHECK((v1) >= (v2))
#define DCHECK_GT(v1, v2) DCHECK((v1) > (v2))

#endif // !__GALILEO_BASE_LOGGING_H__
