#if defined _WIN32 || defined _WIN64
#define BANDLIBRARY_IMPORT __declspec(dllimport)
#elif defined __linux__
#define BANDLIBRARY_IMPORT __attribute__((visibility("default")))
#else
#define BANDLIBRARY_IMPORT
#endif

#include "common.h"
#include "builtin_op_data.h"
