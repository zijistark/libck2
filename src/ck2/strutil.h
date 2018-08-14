#ifndef LIBCK2_STRUTIL_H
#define LIBCK2_STRUTIL_H

#include <cstring>
#include <string_view>

#include "common.h"


NAMESPACE_CK2;
namespace strutil {


// works like linux's strsep(), although implementation may differ. thread-safe way to quickly split a *mutable*
// C-style character string by a given delimeter.
//
// ==> TODO: ultimately should be phased out in favor of a generic tokenizer (templated on char type, delimeter,
// and probably essence parameters such as whether to respect quoting and what type of escape sequences should be
// translated to what) which produces std::string_view objects and does not require a mutable C-style string as input
// (i.e., can work on other string_views or on const pointers to character arrays).
static inline auto strsep(char** sptr, int delim)
{
    auto start = *sptr;

    if (auto p = (start) ? strchr(start, delim) : nullptr) {
        *p = '\0';
        *sptr = p + 1;
    }
    else
        *sptr = nullptr;

    return start;
}


// test if an entire string is effectively empty (literally or just full of blank characters or EOL characters)
static inline constexpr auto is_blank(std::string_view s)
{
    for (const auto& c : s)
        if ( !(c == ' ' || c == '\t' || c == '\n' || c == '\r') )
            return false;

    return true;
}

//// mdh_strncpy

// copy not more than `length` characters from the string `src` (including any NULL terminator) to the string
// `dst` while never overflowing the `dst_sz` memory available to `dst`. return value: actual amount of
// characters copied.

// - precondition: `length <= strlen(src)`; we do not check the actual length ourselves for performance reasons.
// - precondition: `dst_sz` is the max size of the memory for `dst` (not max length but max length + null
//   terminator)
// - precondition: the memory backing `src` and `dst` may not overlap; if it does, UNDEFINED BEHAVIOR! CHAOS!

// - `dst` is always NULL-terminated when done (unlike strncpy)
// - performance of the bounded copy should be virtually as good as you can get.
// --> `dst` will never be unnecessarily padded with O(N) NULLs in a lot of cases (unlike strncpy)
// --> memcpy is intensively optimized for non-overlapping block memory transfer (SIMD, etc.)

// motivation for creation: strncpy is practically obsolete and broken -- but it's also slow due to a poor POSIX
// standardization choice, sprintf and similar are also slower, and of course, strcpy can overflow its output
// buffer.

static inline auto mdh_strncpy(char* dst, size_t dst_sz, const char* const src, size_t length) {
    size_t n = (length > dst_sz) ? dst_sz : length;
    memcpy(dst, src, n);
    dst[n] = '\0'; // only sometimes necessary (when not using as a replacement for strcpy on well-formed input)
    return n;
}


} // end strutil namespace
NAMESPACE_CK2_END;
#endif
