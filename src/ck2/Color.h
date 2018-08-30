#ifndef LIBCK2_COLOR_H
#define LIBCK2_COLOR_H

#include <array>

#include "common.h"


NAMESPACE_CK2;


//TODO: compare default MSVC code generation for `rgb` vs. packing `rgb` to fit within a 32-bit word
//#pragma pack(push, 1)


struct RGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;

  constexpr RGB() : r(0), g(0), b(0) {}

  constexpr RGB(uint red, uint green, uint blue) :
    r(static_cast<uint8_t>(red)),
    g(static_cast<uint8_t>(green)),
    b(static_cast<uint8_t>(blue)) {}

  constexpr RGB(uint rgb) :
    r(static_cast<uint8_t>( (rgb >> 16) & 0xFF )),
    g(static_cast<uint8_t>( (rgb >> 8) & 0xFF )),
    b(static_cast<uint8_t>( rgb & 0xFF )) {}

  constexpr auto red()   const noexcept { return r; }
  constexpr auto green() const noexcept { return g; }
  constexpr auto blue()  const noexcept { return b; }

  constexpr uint32_t to_rgba32() const noexcept // alpha in 'rgba' is 0, of course.
  {
    return static_cast<uint32_t>(red())   << 24 |
           static_cast<uint32_t>(green()) << 16 |
           static_cast<uint32_t>(blue())  << 8;
  }

  constexpr bool operator==(const RGB& c) const noexcept
  {
    return r == c.r &&
           g == c.g &&
           b == c.b;
  }

  constexpr bool operator!=(const RGB& c) const noexcept { return !(*this == c); }
};


struct BGR {
  constexpr BGR()
  : _M_components({0, 0, 0}) {}

  constexpr BGR(uint b, uint g, uint r)
  : _M_components({static_cast<uint8_t>(b), static_cast<uint8_t>(g), static_cast<uint8_t>(r)}) {}

  constexpr BGR(uint8_t const* p)
  : _M_components({p[0], p[1], p[2]}) {}

  constexpr auto blue()  const noexcept { return _M_components[0]; }
  constexpr auto green() const noexcept { return _M_components[1]; }
  constexpr auto red()   const noexcept { return _M_components[2]; }

  constexpr uint8_t const* components() const noexcept { return &_M_components[0]; }

  constexpr uint32_t to_bgra32() const noexcept // alpha in 'bgra' is 0, of course.
  {
    return static_cast<uint32_t>(blue())  << 24 |
           static_cast<uint32_t>(green()) << 16 |
           static_cast<uint32_t>(red())   << 8;
  }

  constexpr bool operator==(const BGR& c) const noexcept
  {
    return _M_components[0] == c._M_components[0] &&
           _M_components[1] == c._M_components[1] &&
           _M_components[2] == c._M_components[2];
  }

  constexpr bool operator!=(const BGR& c) const noexcept { return !(*this == c); }

private:
  std::array<uint8_t, 3> _M_components;
};

//#pragma pack(pop)

NAMESPACE_CK2_END;

/* inject std::hash<rgb> specialization */

namespace std {
  // TODO: try to find an actual 24-bit hash or something that will help improve this hash in an RGB context or,
  // rather, just fiddle with FNV-1a vs. other popular functions and see what happens. the most obvious problem
  // is that all the colors have their 8 MSBs all zeroed.

  template<> struct hash<ck2::RGB> {
    size_t operator()(const ck2::RGB& c) const noexcept { return std::hash<uint32_t>{}( c.to_rgba32() ); }
  };

  template<> struct hash<ck2::BGR> {
    size_t operator()(const ck2::BGR& c) const noexcept { return std::hash<uint32_t>{}( c.to_bgra32() ); }
  };
}

#endif
