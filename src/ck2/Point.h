#ifndef LIBCK2_POINT_H
#define LIBCK2_POINT_H

#include <type_traits>

#include "common.h"


NAMESPACE_CK2;


#pragma pack(push, 1) // We'd like our Point objects to be packed as tightly as possible in memory.

template<typename CoordT = uint16_t>
struct Point {
  using value_type = CoordT;

  CoordT x;
  CoordT y;

  constexpr Point()
    : x(0)
    , y(0) {}

  constexpr Point(CoordT x_, CoordT y_)
    : x(x_)
    , y(y_) {}

  constexpr auto operator==(Point p) const noexcept
  {
    static_assert(!std::is_floating_point<CoordT>::value,
                  "Floating-point coordinate types for ck2::Point don't support equality tests by default.");

    return x == p.x &&
           y == p.y;
  }

  constexpr auto operator!=(Point p) const noexcept { return !(*this == p); }

  static_assert(std::is_arithmetic<CoordT>::value && std::is_trivial<CoordT>::value,
                "Coordinate type for ck2::Point must be arithmetic and trivially copyable/constructible.");

  static_assert(sizeof(CoordT) * 2 <= 16, // we pass Point by value by default, so huge Points will be slow.
                "For performance reasons, the size of a ck2::Point should be no greater than 16 bytes.");
};

#pragma pack(pop)

// Should we ever we need points of a different rank (e.g., a Point3D) in this
// library, the nominal "Point" will still always be 2D.

template<typename CoordT>
using Point2D = Point<CoordT>;


NAMESPACE_CK2_END;


namespace std {
  template<> struct hash<ck2::Point<uint8_t>> {
    size_t operator()(ck2::Point<uint8_t> p) const noexcept
    {
      return hash<uint8_t>{}(p.x) ^ hash<uint8_t>{}(p.y) << 1;
    }
  };
  template<> struct hash<ck2::Point<int8_t>> {
    size_t operator()(ck2::Point<int8_t> p) const noexcept
    {
      return hash<int8_t>{}(p.x) ^ hash<int8_t>{}(p.y) << 1;
    }
  };
  template<> struct hash<ck2::Point<uint16_t>> {
    size_t operator()(ck2::Point<uint16_t> p) const noexcept
    {
      return hash<uint16_t>{}(p.x) ^ hash<uint16_t>{}(p.y) << 1;
    }
  };
  template<> struct hash<ck2::Point<int16_t>> {
    size_t operator()(ck2::Point<int16_t> p) const noexcept
    {
      return hash<int16_t>{}(p.x) ^ hash<int16_t>{}(p.y) << 1;
    }
  };
  template<> struct hash<ck2::Point<uint32_t>> {
    size_t operator()(ck2::Point<uint32_t> p) const noexcept
    {
      return hash<uint32_t>{}(p.x) ^ hash<uint32_t>{}(p.y) << 1;
    }
  };
  template<> struct hash<ck2::Point<int32_t>> {
    size_t operator()(ck2::Point<int32_t> p) const noexcept
    {
      return hash<int32_t>{}(p.x) ^ hash<int32_t>{}(p.y) << 1;
    }
  };
  template<> struct hash<ck2::Point<float>> {
    size_t operator()(ck2::Point<float> p) const noexcept
    {
      return hash<float>{}(p.x) ^ hash<float>{}(p.y) << 1;
    }
  };
  template<> struct hash<ck2::Point<double>> {
    size_t operator()(ck2::Point<double> p) const noexcept
    {
      return hash<double>{}(p.x) ^ hash<double>{}(p.y) << 1;
    }
  };
};


#endif
