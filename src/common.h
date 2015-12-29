#ifndef COMMON_H
#define COMMON_H

#define NELEM(x) (sizeof(x)/sizeof(x[0]))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define DEBUG

#ifdef DEBUG
#define APP_DEBUG(...) APP_LOG(APP_LOG_LEVEL_DEBUG, __VA_ARGS__)
#else
#define APP_DEBUG(...)
#endif

#endif
