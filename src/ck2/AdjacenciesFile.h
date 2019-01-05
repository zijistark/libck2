#ifndef LIBCK2_ADJACENCIES_FILE_H
#define LIBCK2_ADJACENCIES_FILE_H

#include "common.h"
#include "filesystem.h"
#include <string_view>
#include <string>
#include <vector>


NAMESPACE_CK2;


class VFS;
class DefaultMap;


class AdjacenciesFile {
public:
  struct Adjacency {
    uint from;
    uint to;
    uint through;
    std::string type;
    std::string comment;
    bool deleted;

    Adjacency(uint _from, uint _to, uint _through, std::string_view _type, std::string_view _comment)
      : from(_from), to(_to), through(_through), type(_type), comment(_comment), deleted(false) {}
  };

public:
  AdjacenciesFile() {}
  AdjacenciesFile(const VFS&, const DefaultMap&);
  void write(const fs::path&);

  /* give this type a container-like interface and C++11 range-based-for support */
  auto size()  const noexcept { return _v.size(); }
  auto empty() const noexcept { return (size() == 0); }
  auto begin()       noexcept { return _v.begin(); }
  auto end()         noexcept { return _v.end(); }
  auto begin() const noexcept { return _v.cbegin(); }
  auto end()   const noexcept { return _v.cend(); }

  // TODO: should have methods for adding entries, removing them, etc. that are forwarded to underlying vector.
  // same goes for DefinitionsTable.

private:
  std::vector<Adjacency> _v;
};


NAMESPACE_CK2_END;
#endif
