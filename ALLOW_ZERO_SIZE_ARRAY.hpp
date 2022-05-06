// ALLOW_ZERO_SIZE_ARRAY(...) macro
// silence -pedantic warnings on use of zero-size array type T[0]

// #include this header twice, sandwiching the code to silence.
// This toggles the macro definition; 1st #define, 2nd #undef
/*
    #include "ALLOW_ZERO_SIZE_ARRAY.hpp"
    ALLOW_ZERO_SIZE_ARRAY(int a[0];)
    #include "ALLOW_ZERO_SIZE_ARRAY.hpp"
*/

#if !defined(ALLOW_ZERO_SIZE_ARRAY)

#if defined(__clang__) || defined(__GNUG__)
#   define ALLOW_ZERO_SIZE_ARRAY(...) \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wpedantic\"") \
__VA_ARGS__ \
_Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#   define ALLOW_ZERO_SIZE_ARRAY(...) \
__pragma(warning(push)) \
__pragma(warning(disable:6246)) \
__VA_ARGS__ \
__pragma(warning(pop))
#else
#   define ALLOW_ZERO_SIZE_ARRAY(...)
#endif

#else
# undef ALLOW_ZERO_SIZE_ARRAY

#endif
