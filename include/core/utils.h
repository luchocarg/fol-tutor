#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#include <stdio.h>

#define SAFE_APPEND(p, end, ...)                                               \
  do {                                                                         \
    if ((p) < (end)) {                                                         \
      int written = snprintf((p), (size_t)((end) - (p)), __VA_ARGS__);         \
      if (written > 0) {                                                       \
        if ((size_t)written >= (size_t)((end) - (p))) {                        \
          (p) = (end);                                                         \
        } else {                                                               \
          (p) += written;                                                      \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)

#endif