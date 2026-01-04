#pragma once

#include <stdio.h>

// Define DEBUG in your build or uncomment below to enable verbose logs.
#define DEBUG 1

#ifdef DEBUG
#define LOGF(...) printf(__VA_ARGS__)
#else
#define LOGF(...) do {} while (0)
#endif

#define ERRF(...) printf(__VA_ARGS__)
