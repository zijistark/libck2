#ifndef LIBCK2_POINT_H
#define LIBCK2_POINT_H

#include <type_traits>

#include "common.h"


NAMESPACE_CK2;


#pragma pack(push, 1) // We'd like our Point2D objects to be packed as tightly as possible in memory.

template<typename CoordT = uint16_t>
struct Point2D {
  // Yes, users are expected to directly access Point2D's coordinate member variables.
  CoordT x;
  CoordT y;

  static_assert(std::is_arithmetic<CoordT>::value && std::is_trivial<CoordT>::value,
                "Coordinate type for ck2::Point2D must be arithmetic and trivially copyable/constructible!");

  static_assert(sizeof(CoordT) <= 16, // since whenever we've got a say in the matter, we pass Point2D by value
                "For performance reasons, the size of ck2::Point2D's coordinate type shouldn't exceed 128-bit.");

  constexpr Point2D() : x(0), y(0) {}
  constexpr Point2D(CoordT x_, CoordT y_) : x(x_), y(y_) {}

  // The concept of a null coordinate or a null Point2D is totally optional for users of this class; if they've no
  // need for a nullable Point2D, then they can freely use the maximum value for the coordinate type.
  static constexpr auto NullCoord() noexcept { return std::numeric_limits<CoordT>::max(); }
  static constexpr auto Null()      noexcept { return Point2D{ NullCoord(), NullCoord() }; }

  // NOTE: Returns true if either coordinate is null (again, ignore this if you don't care about nullable points)
  constexpr auto is_null() const noexcept { return x == NullCoord() || y == NullCoord(); }

  // TODO: Equality/inequality/comparison operators should use C++17 constexpr-if (or just std::enable_if SFINAE)
  // to differentiate their implementation for floating-point coordinate types (inexact (in)equality required for
  // floating-point, as much of a bitch as it is to handle due to FP's nonlinear precision over its dynamic range).

  // MAYBE-TODO: Disable modulo & bit shift operations on non-integral coordinate types. Indeed, probably force the
  // types to be unsigned where necessary to avoid UB with these operations as well. Look into the specifics of
  // C++17's notion of platform-independent/defined behavior regarding floating-point modulo & whether we've yet
  // [finally] set a better standard for defined behavior regarding modulo with signed integers.

  // MAYBE-TODO: Add unary arithmetic negation

  // MAYBE-TODO: Add bitwise AND, OR, and XOR operations. Also add unary bitwise complement (NOT).

  // MAYBE-TODO: Add conversion to & from Z-order representation (interleaved bits from each coordinate), as a
  // helper method (implemented via the above bitwise operations).

  // MAYBE-TODO: Make it easier to perform geometry & trigonometry transformations on Point2Ds (e.g., rotate a
  // Point2D about the origin by an angle or calculate the Euclidean distance between 2 Point2Ds or simply
  // calculate the "power" / hypot of a single Point2D).

  constexpr auto operator==(Point2D pt) const noexcept { return x == pt.x && y == pt.y; }
  constexpr auto operator!=(Point2D pt) const noexcept { return !(*this == pt); }
  constexpr auto operator<(Point2D pt)  const noexcept { return (x == pt.x) ? y < pt.y : x < pt.x; }
  constexpr auto operator>(Point2D pt)  const noexcept { return (x == pt.x) ? y > pt.y : x > pt.x; }
  constexpr auto operator>=(Point2D pt) const noexcept { return !(*this < pt); }
  constexpr auto operator<=(Point2D pt) const noexcept { return !(*this > pt); }
  // HAHA-TODO: Gimme my C++20 spaceship operator (three-way comparison)!

  constexpr auto& operator=(Point2D pt)   noexcept { x = pt.x; y = pt.x; return *this; }
  constexpr auto& operator+=(Point2D pt)  noexcept { x += pt.x; y += pt.y; return *this; }
  constexpr auto& operator-=(Point2D pt)  noexcept { x -= pt.x; y -= pt.y; return *this; }
  constexpr auto& operator*=(Point2D pt)  noexcept { x *= pt.x; y *= pt.y; return *this; }
  constexpr auto& operator/=(Point2D pt)  noexcept { x /= pt.x; y /= pt.y; return *this; }
  constexpr auto& operator%=(Point2D pt)  noexcept { x %= pt.x; y %= pt.y; return *this; }
  constexpr auto& operator>>=(Point2D pt) noexcept { x >>= pt.x; y >>= pt.y; return *this; }
  constexpr auto& operator<<=(Point2D pt) noexcept { x <<= pt.x; y <<= pt.y; return *this; }

  constexpr auto& operator=(CoordT c)   noexcept { x = c; y = c; return *this; }
  constexpr auto& operator+=(CoordT c)  noexcept { x += c; y += c; return *this; }
  constexpr auto& operator-=(CoordT c)  noexcept { x -= c; y -= c; return *this; }
  constexpr auto& operator*=(CoordT c)  noexcept { x *= c; y *= c; return *this; }
  constexpr auto& operator/=(CoordT c)  noexcept { x /= c; y /= c; return *this; }
  constexpr auto& operator%=(CoordT c)  noexcept { x %= c; y %= c; return *this; }
  constexpr auto& operator>>=(CoordT c) noexcept { x >>= c; y >>= c; return *this; }
  constexpr auto& operator<<=(CoordT c) noexcept { x <<= c; y <<= c; return *this; }

  // prefix incr/decr
  constexpr auto& operator++() noexcept { ++x; ++y; return *this; }
  constexpr auto& operator--() noexcept { --x; --y; return *this; }

  // postfix incr/decr
  constexpr auto operator++(int) noexcept { auto old = *this; ++(*this); return old; }
  constexpr auto operator--(int) noexcept { auto old = *this; --(*this); return old; }

  constexpr auto operator+(Point2D pt)  const noexcept { return Point2D(x + pt.x, y + pt.y); }
  constexpr auto operator-(Point2D pt)  const noexcept { return Point2D(x - pt.x, y - pt.y); }
  constexpr auto operator*(Point2D pt)  const noexcept { return Point2D(x * pt.x, y * pt.y); }
  constexpr auto operator/(Point2D pt)  const noexcept { return Point2D(x / pt.x, y / pt.y); }
  constexpr auto operator>>(Point2D pt) const noexcept { return Point2D(x >> pt.x, y >> pt.y); }
  constexpr auto operator<<(Point2D pt) const noexcept { return Point2D(x << pt.x, y << pt.y); }

  constexpr auto operator+(CoordT c)  const noexcept { return Point2D(x + c, y + c); }
  constexpr auto operator-(CoordT c)  const noexcept { return Point2D(x - c, y - c); }
  constexpr auto operator*(CoordT c)  const noexcept { return Point2D(x * c, y * c); }
  constexpr auto operator/(CoordT c)  const noexcept { return Point2D(x / c, y / c); }
  constexpr auto operator%(CoordT c)  const noexcept { return Point2D(x % c, y % c); }
  constexpr auto operator>>(CoordT c) const noexcept { return Point2D(x >> c, y >> c); }
  constexpr auto operator<<(CoordT c) const noexcept { return Point2D(x << c, y << c); }

  friend constexpr auto operator+(CoordT c, Point2D pt)  noexcept { return Point2D(c + pt.x, c + pt.y); }
  friend constexpr auto operator-(CoordT c, Point2D pt)  noexcept { return Point2D(c - pt.x, c - pt.y); }
  friend constexpr auto operator*(CoordT c, Point2D pt)  noexcept { return Point2D(c * pt.x, c * pt.y); }
  friend constexpr auto operator/(CoordT c, Point2D pt)  noexcept { return Point2D(c / pt.x, c / pt.y); }
  friend constexpr auto operator%(CoordT c, Point2D pt)  noexcept { return Point2D(c % pt.x, c % pt.y); }
  friend constexpr auto operator>>(CoordT c, Point2D pt) noexcept { return Point2D(c >> pt.x, c >> pt.y); }
  friend constexpr auto operator<<(CoordT c, Point2D pt) noexcept { return Point2D(c << pt.x, c << pt.y); }
};

#pragma pack(pop)

// Should we ever we need points of a different rank (e.g., a Point3D) in this library, the nominal "Point" will
// still always be 2D.
using Point = Point2D;


END_NAMESPACE_CK2;


namespace std {
  template<> struct hash< ck2::Point<uint8_t> > {
    size_t operator()(ck2::Point<uint8_t> p) const noexcept
    {
      const auto hx = hash<uint8_t>{}(p.x);
      const auto hy = hash<uint8_t>{}(p.y);
      return hx ^ hy << 1;
    }
  };
  template<> struct hash< ck2::Point<int8_t> > {
    size_t operator()(ck2::Point<int8_t> p) const noexcept
    {
      const auto hx = hash<int8_t>{}(p.x);
      const auto hy = hash<int8_t>{}(p.y);
      return hx ^ hy << 1;
    }
  };
  template<> struct hash< ck2::Point<uint16_t> > {
    size_t operator()(ck2::Point<uint16_t> p) const noexcept
    {
      const auto hx = hash<uint16_t>{}(p.x);
      const auto hy = hash<uint16_t>{}(p.y);
      return hx ^ hy << 1;
    }
  };
  template<> struct hash< ck2::Point<int16_t> > {
    size_t operator()(ck2::Point<int16_t> p) const noexcept
    {
      const auto hx = hash<int16_t>{}(p.x);
      const auto hy = hash<int16_t>{}(p.y);
      return hx ^ hy << 1;
    }
  };
  template<> struct hash< ck2::Point<uint32_t> > {
    size_t operator()(ck2::Point<uint32_t> p) const noexcept
    {
      const auto hx = hash<uint32_t>{}(p.x);
      const auto hy = hash<uint32_t>{}(p.y);
      return hx ^ hy << 1;
    }
  };
  template<> struct hash< ck2::Point<int32_t> > {
    size_t operator()(ck2::Point<int32_t> p) const noexcept
    {
      const auto hx = hash<int32_t>{}(p.x);
      const auto hy = hash<int32_t>{}(p.y);
      return hx ^ hy << 1;
    }
  };
  template<> struct hash< ck2::Point<float> > {
    size_t operator()(ck2::Point<float> p) const noexcept
    {
      const auto hx = hash<float>{}(p.x);
      const auto hy = hash<float>{}(p.y);
      return hx ^ hy << 1;
    }
  };
  template<> struct hash< ck2::Point<double> > {
    size_t operator()(ck2::Point<double> p) const noexcept
    {
      const auto hx = hash<double>{}(p.x);
      const auto hy = hash<double>{}(p.y);
      return hx ^ hy << 1;
    }
  };
};


#endif
