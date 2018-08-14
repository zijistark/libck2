#ifndef LIBCK2_FILE_LOCATION_H
#define LIBCK2_FILE_LOCATION_H

#include "common.h"
#include "Error.h"
#include "Location.h"
#include "filesystem.h"


NAMESPACE_CK2;


class FileLocation : public Location {
  using Base = Location;
public:
  FileLocation(const fs::path& path_, const Location& loc_) : Base(loc_), _path(path_) {}
  FileLocation(const fs::path& path_, uint line_ = 0, uint col_ = 0) : Base(line_, col_), _path(path_) {}

  auto const& path() const noexcept { return _path; }
  auto&       path()       noexcept { return _path; }

  auto to_string() const
  {
    auto ls = Base::to_string();
    return ls.empty() ? _path.generic_string() : _path.generic_string() + ":" + ls;
  }

  auto to_string_prefix() const { return to_string() + ": "; }

  auto to_string_suffix() const
  {
    return Base::to_string_suffix() + fmt::format(" in '{}'", _path.generic_string());
  }

private:
  fs::path _path;
};


using FLoc = FileLocation;


class FLError : public Error {
  using Base = Error;
public:
  auto const& floc() const noexcept { return _fl; }
  auto&       floc()       noexcept { return _fl; }

  FLError(const FLoc& fl_, const std::string& msg)
    : Base(fl_.to_string_prefix() + msg), _fl(fl_) {}

  template<typename... Args>
  FLError(const FLoc& fl_, std::string_view format, const Args& ...args)
    : Base(fl_.to_string_prefix() + fmt::vformat(format, fmt::make_format_args(args...))), _fl(fl_) {}

private:
  FLoc _fl;
};


// binds a lambda/functor which produces the current path/line/column (floc) on-demand to a factory which returns
// appropriate FLError objects (presumably to throw).  this is purely for improving convenience & reducing
// redunancy in code that may throw a lot of FLError-style exceptions.
template<typename FLocFuncT>
struct FLErrorFactory
{
  constexpr FLErrorFactory(FLocFuncT& fl_func_)
    : _fl_func(fl_func_) {}

  template<typename... Args>
  constexpr auto operator()(std::string_view format, Args&& ...args) const
  {
    return FLError(_fl_func(), format, std::forward<Args>(args)...);
  }

private:
  FLocFuncT& _fl_func;
};


// when the FileLocation doesn't actually change (typically for when omitting line/col information, such as in
// binary files)
struct FLErrorStaticFactory
{
  FLErrorStaticFactory(const FLoc& fl_) : _fl(fl_) {}

  template<typename... Args>
  constexpr auto operator()(std::string_view format, Args&& ...args) const
  {
    return FLError(_fl, format, std::forward<Args>(args)...);
  }

private:
  FLoc _fl;
};


NAMESPACE_CK2_END;
#endif
