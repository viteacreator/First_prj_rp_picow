#pragma once

#include <stdio.h>

// Debug levels: 0=off, 1=error, 2=warn, 3=info, 4=debug
// #define DEBUG_LEVEL 4

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 3
#endif

#define LOGE(...) do { if (DEBUG_LEVEL >= 1) printf(__VA_ARGS__); } while (0)
#define LOGW(...) do { if (DEBUG_LEVEL >= 2) printf(__VA_ARGS__); } while (0)
#define LOGI(...) do { if (DEBUG_LEVEL >= 3) printf(__VA_ARGS__); } while (0)
#define LOGD(...) do { if (DEBUG_LEVEL >= 4) printf(__VA_ARGS__); } while (0)

#define ERRF(...) LOGE(__VA_ARGS__)
#define LOGF(...) LOGI(__VA_ARGS__)
