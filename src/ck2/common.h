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


NAMESPACE_CK2_END;
#endif
