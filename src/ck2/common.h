#ifndef LIBCK2_COMMON_H
#define LIBCK2_COMMON_H


#define NAMESPACE_CK2 namespace ck2 {
#define NAMESPACE_CK2_END }


#include <cstdint>
#include <cassert>
#include <iosfwd>
#include <limits>

// TODO: remove these from this common header soon:
#include <cstring>
#include "Error.h"


/* a user-defined literal suffix for unsigned short, aka a "half word" */
inline constexpr std::uint16_t operator"" _h(unsigned long long value) noexcept
{
  return static_cast<std::uint16_t>(value);
}


NAMESPACE_CK2;


typedef unsigned int uint;
using prov_id_t = uint16_t;
using char_id_t = int32_t;

#define LIBCK2_VERSION_MAJOR "0"
#define LIBCK2_VERSION_MINOR "10"
#define LIBCK2_VERSION_PATCH "0"
#define LIBCK2_VERSION_BUILD "" // might be, e.g., "-rc3" or the like in the future
#define LIBCK2_VERSION_MAJOR_N (0)
#define LIBCK2_VERSION_MINOR_N (10)
#define LIBCK2_VERSION_PATCH_N (0)
#define LIBCK2_VERSION_STRING "v" LIBCK2_VERSION_MAJOR "." LIBCK2_VERSION_MINOR "." LIBCK2_VERSION_PATCH LIBCK2_VERSION_BUILD

NAMESPACE_CK2_END;
#endif
