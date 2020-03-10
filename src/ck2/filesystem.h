#ifndef LIBCK2_FILESYSTEM_H
#define LIBCK2_FILESYSTEM_H

#include <cstdio>
#include <memory>
#include <string>
#include <filesystem>

#include "common.h"


// this header exists foremost as a compilation proxy between the different not-quite-there-yet C++17
// implementations of <filesystem> (or <experimental/filesytem>) and Boost.

// after that, it's convenience stuff for users of FS-related things.


NAMESPACE_CK2;


namespace fs = std::filesystem;
using unique_file_ptr = std::unique_ptr< std::FILE, int (*)(std::FILE*) >;


struct PathError : public Error {
  PathError(const std::string& msg_, const fs::path& path_) : Error(msg_), _path(path_) {}
  auto&       path()       noexcept { return _path; }
  const auto& path() const noexcept { return _path; }

protected:
  fs::path _path;
};


struct PathNotFoundError : public PathError {
  PathNotFoundError(const fs::path& path_)
    : PathError(fmt::format("Path not found: {}", path_.generic_string()), path_) {}
};


struct PathTypeError : public PathError {
  PathTypeError(const fs::path& path_) // TODO: tell the user what type of file it does point to vs. expected
    : PathError(fmt::format("Path points to unexpected file type (e.g., directory vs. regular file): {}",
                path_.generic_string()), path_) {}
};


NAMESPACE_CK2_END;
#endif
