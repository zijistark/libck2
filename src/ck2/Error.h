#ifndef LIBCK2_ERROR_H
#define LIBCK2_ERROR_H


#include "fmt/format.h"
#include <string_view>
#include <exception>
#include <stdexcept>


NAMESPACE_CK2;


class Error : public std::runtime_error {
  using Base = std::runtime_error;

public:
  Error(const std::string& msg) : Base(msg) {}

  // Error(const char* format, fmt::format_args args)
  //     : std::runtime_error( fmt::vformat(format, args) ) {}

  template<typename... Args>
  Error(std::string_view format, const Args& ...args)
    : Base( fmt::vformat(format, fmt::make_format_args(args...) ) ) {}
};


NAMESPACE_CK2_END;
#endif
