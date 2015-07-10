#pragma once

#ifdef ENABLE_DOG_DEBUG

# ifdef __cplusplus
#  include <cstdio>
#  define DOG_PRINT(...) std::fprintf(stderr, __VA_ARGS__)
# else
#  include <stdio.h>
#  define DOG_PRINT(...) fprintf(stderr, __VA_ARGS__)
# endif

#else
#  define DOG_PRINT(...)
#endif
