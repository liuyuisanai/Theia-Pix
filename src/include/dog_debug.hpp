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

/* Prevent unused-variable errors and warnings while debug is disabled. */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
inline void
DOG_PRINT(const char fmt[], ...) {}
#pragma GCC diagnostic pop

#endif
