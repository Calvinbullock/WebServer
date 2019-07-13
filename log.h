#include <stdio.h>

#define __STRINGIZE(X) __DO_STRINGIZE(X)
#define __DO_STRINGIZE(X) #X

#define __LOG(level, format, ...)                                              \
  printf(level "(" __FILE__ ":" __STRINGIZE(__LINE__) "): " format "\n",       \
         ##__VA_ARGS__)

#define LOG_INFO(format, ...) __LOG("INFO", format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) __LOG("DEBUG", format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) __LOG("WARNING", format, ##__VA_ARGS__)
