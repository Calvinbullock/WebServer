#include <stdio.h>

#define LOG__(level, format, ...)                                              \
  printf(level "(" __FILE__ "): " format "\n", ##__VA_ARGS__)

#define LOG_INFO(format, ...) LOG__("INFO", format, ##__VA_ARGS__)

#define LOG_DEBUG(format, ...) LOG__("DEBUG", format, ##__VA_ARGS__)
