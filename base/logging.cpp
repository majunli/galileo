#include "logging.h"

#include <cstdarg>
#include <Windows.h>

namespace galileo {

Logger::~Logger() {
  int n, size = 64;
  char *text;
  if ((text = (char *)malloc(size)) != NULL) {
    while (1) {
      n = _snprintf_s(text, size, size, "%s:%i: %s", file_, line_, message_.c_str());
      if (n > -1 && n < size) {
        message_ = text;
        break;
      }
      size *= 2;
      if ((text = (char *)realloc(text, size)) == NULL)
        break;
    }
    free(text);
  }
  OutputDebugStringA(message_.c_str());
}

void Logger::printf(const char* fmt, ...) {
  int n, size = 64;
  char *text;
  if ((text = (char *)malloc(size)) != NULL) {
    va_list args;
    while (1) {
      va_start(args, fmt);
      n = vsnprintf_s(text, size, size, fmt, args);
      va_end(args);
      if (n > -1 && n < size) {
        message_.append(text);
        break;;
      }
      size *= 2;
      if ((text = (char *)realloc(text, size)) == NULL)
        break;
    }
    free(text);
  }
}

} // namespace galileo

galileo::Logger& operator<<(galileo::Logger& log, const char* s) {
  log.printf("%s", s);
  return log;
}

galileo::Logger& operator<<(galileo::Logger& log, const std::string& s) {
  log.printf("%s", s.c_str());
  return log;
}

galileo::Logger& operator<<(galileo::Logger& log, int i) {
  log.printf("%i", i);
  return log;
}

galileo::Logger& operator<<(galileo::Logger& log, void* p) {
  log.printf("%p", p);
  return log;
}

