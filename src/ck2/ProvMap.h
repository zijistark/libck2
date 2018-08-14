#ifndef LIBCK2_PROVINCE_MAP_H
#define LIBCK2_PROVINCE_MAP_H

#include <limits>

#include "common.h"
#include "filesystem.h"


NAMESPACE_CK2;


class DefinitionsTable;
class DefaultMap;
class VFS;


// A ProvMap class opens & processes the raw bitmap from 'provinces.bmp' & allocates it into a buffer(s) of
// 16-bit province IDs. It then provides read-only access to that mapping of IDs to raster (x, y) grid coordinates.
// Some province IDs at the very top of the ID range are reserved for useful classifications.

// API NOTE: Never assume the actual memory representation of the grid is indeed one big, rectangular grid. Ergo,
// always access grid contents via the operator(x, y) overload. [didn't feel like dealing with proxy overloads for
// operator[].]
//
// there will probably be a faster, layout-aware means of traversing the grid later.
struct ProvMap
{
    ProvMap(const VFS&, const DefaultMap&, const DefinitionsTable&);

    // We always maintain 0 as the 'null province.' For ProvMap, this actually doesn't matter for now, so this is
    // practically documentation.
    static constexpr prov_id_t PM_NULL = 0;

    // But these definitely matter right now for ProvMap:
    static constexpr prov_id_t PM_IMPASSABLE  = std::numeric_limits<prov_id_t>::max();
    static constexpr prov_id_t PM_OCEAN       = std::numeric_limits<prov_id_t>::max() - 1;
    static constexpr prov_id_t PM_REAL_ID_MAX = std::numeric_limits<prov_id_t>::max() - 2;

    auto width()  const noexcept { return _cols; }
    auto height() const noexcept { return _rows; }

    auto& operator()(uint x, uint y)       noexcept { return _map[ y * _cols + x ]; }
    auto& operator()(uint x, uint y) const noexcept { return _map[ y * _cols + x ]; }

    auto data()       noexcept { return _map.get(); }
    auto data() const noexcept { return _map.get(); }

private:
    std::unique_ptr<prov_id_t[]> _map;
    uint _cols;
    uint _rows;
};


NAMESPACE_CK2_END;
#endif
