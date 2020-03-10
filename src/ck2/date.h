#ifndef LIBCK2_DATE_H
#define LIBCK2_DATE_H

#include "common.h"

#include <cstdlib>
#include <limits>


NAMESPACE_CK2;


#ifdef _MSC_VER
  #pragma pack(push, 1)
#endif

struct date
{
  date(char* src); // only for use on mutable strings known to be well-formed (typically due to being tokenized)

  date(int year = 1, uint month = 1, uint day = 1)
  : _y(year)
  , _m(month)
  , _d(day)
  {
#ifdef DEBUG_MAX
    assert( year != 0 && year >= std::numeric_limits<int16_t>::min() && year <= std::numeric_limits<int16_t>::max() );
    assert( month > 0 && month <= 12 );
    assert( day > 0 && day <= 31 );
#endif
  }

  int  year()  const noexcept { return _y; }
  uint month() const noexcept { return _m; }
  uint day()   const noexcept { return _d; }

  bool operator<(const date& o) const noexcept
  {
    // FIXME: may require rethinking for inequal years BC
    if (_y < o._y) return true;
    if (o._y < _y) return false;
    if (_m < o._m) return true;
    if (o._m < _m) return false;
    if (_d < o._d) return true;
    if (o._d < _d) return false;
    return false;
  }

  bool operator==(const date& o) const noexcept { return _y == o._y && _m == o._m && _d == o._d; }
  bool operator>=(const date& o) const noexcept { return !(*this < o); }
  bool operator!=(const date& o) const noexcept { return !(*this == o); }
  bool operator> (const date& o) const noexcept { return *this >= o && *this != o; }
  bool operator<=(const date& o) const noexcept { return *this < o || *this == o; }

  friend std::ostream& operator<<(std::ostream& os, date d)
  {
    return os << d.year() << '.' << d.month() << '.' << d.day();
  }

private:
  int16_t _y;
  uint8_t _m;
  uint8_t _d;
}

#ifndef _MSC_VER
  __attribute__ ((packed))
#endif
; // close the class definition statement

#ifdef _MSC_VER
  #pragma pack(pop)
#endif


NAMESPACE_CK2_END;
#endif
