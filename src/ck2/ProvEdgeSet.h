#ifndef LIBCK2_PROV_EDGE_SET_H
#define LIBCK2_PROV_EDGE_SET_H

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "ProvEdge.h"
#include "Point.h"
#include "common.h"


NAMESPACE_CK2;


struct ProvMap;

struct ProvEdgeSet {
  ProvEdgeSet(const ProvMap&);

  auto cbegin()  const noexcept { return _M_edges.cbegin(); }
  auto cend()    const noexcept { return _M_edges.cend(); }
  auto begin()   const noexcept { return cbegin(); }
  auto end()     const noexcept { return cend(); }
  auto begin()         noexcept { return _M_edges.begin(); }
  auto end()           noexcept { return _M_edges.end(); }

  auto crbegin() const noexcept { return _M_edges.crbegin(); }
  auto crend()   const noexcept { return _M_edges.crend(); }
  auto rbegin()  const noexcept { return crbegin(); }
  auto rend()    const noexcept { return crend(); }
  auto rbegin()        noexcept { return _M_edges.rbegin(); }
  auto rend()          noexcept { return _M_edges.rend(); }

  auto size()    const noexcept { return _M_edges.size(); }
  auto empty()   const noexcept { return size() == 0; }

  auto& front() const noexcept { return _M_edges.front(); }
  auto& front()       noexcept { return _M_edges.front(); }
  auto& back()  const noexcept { return _M_edges.back(); }
  auto& back()        noexcept { return _M_edges.back(); }

private:
  using P16 = Point<uint16_t>;

  // LPoint has a coordinate type that's local to this ProvEdgeSet (might not be the same as the global
  // coordinate type in the future).
  using LPoint  = P16;
  using coord_t = LPoint::value_type;
  using id_pair = std::pair<prov_id_t, prov_id_t>;

#pragma pack(push, 1)

  struct Segment {

    enum class Direction
    {
      Vertical, // x-coord is fixed
      Horizontal, // y-coord is fixed
    };

    constexpr Segment()
      : start(0)
      , end(0) {}

    constexpr Segment(id_pair relation_, coord_t start_)
      : relation(relation_)
      , start(start_)
      , end(static_cast<coord_t>(start_ + 1)) {}

    constexpr operator bool() const noexcept { return end != 0; }
    constexpr void reset() noexcept { end = 0; }

    id_pair relation;
    coord_t start;
    coord_t end;
  };

#pragma pack(pop)

  using Direction   = Segment::Direction;
  using EndpointMap = std::unordered_multimap< LPoint, uint >;

  auto try_edge(
    Direction dir,
    coord_t fixed_coord,
    coord_t coord,
    id_pair rel,
    Segment& seg,
    EndpointMap& map)
  {
    if (seg)
    {
      if (seg.relation == rel)
      {
        // non-null segment & compatible edge, so extend current segment
        ++seg.end;
        return true;
      }
      else
      {
        // non-null segment & not a compatible edge, so finish segment
        finish_segment(dir, fixed_coord, seg, map);
        seg.reset();
      }
    }

    if (!seg && rel.first != rel.second)
    {
      // have an edge, yet we've a null segment [possibly just became so a few instructions ago due to edge
      // relation incompatibility], so start a new segment here.
      seg = Segment(rel, coord);
      return true;
    }

    return false;
  }

  auto try_vertical_edge(coord_t x, coord_t y, const prov_id_t* p, Segment& seg, EndpointMap& map)
  {
    return try_edge(Direction::Vertical,
                    x,
                    y,
                    std::make_pair(*p, *(p + 1)),
                    seg,
                    map);
  }

  auto try_horizontal_edge(coord_t x, coord_t y, const prov_id_t* p, Segment& seg, EndpointMap& map) {
    return try_edge(Direction::Horizontal,
                    y,
                    x,
                    std::make_pair(*p, *(p + _M_width)),
                    seg,
                    map);
  }

  void finish_segment(Direction, coord_t fixed_coord, const Segment&, EndpointMap&);

  enum class EdgeEnd
  {
    Front,
    Back,
  };

  void trace_edge(uint edge_idx, EndpointMap& map)
  {
    trace_edge_end(EdgeEnd::Front, edge_idx, map);
    trace_edge_end(EdgeEnd::Back, edge_idx, map);
  }

  void trace_edge_end(const EdgeEnd, uint edge_idx, EndpointMap&);

  std::vector< std::unique_ptr< ProvEdge > > _M_edges;

  const prov_id_t* _M_grid; // row-major order
  coord_t          _M_width;
  coord_t          _M_height;


#ifdef DEBUG
  // Statistics toward the end of improving these algorithms:
  uint _M_paraxial_seg_count; // count of all paraxial edge segments generated before promoting to ProvEdges
  uint _M_paraxial_seg_max; // theoretical max count based upon width & height
  uint _M_paraxial_seg_len_sum; // sum of all paraxial edge segment lengths (with count, we derive an average)

  // Same as above but it's the number of unit edges saved from being promoted to ProvEdges by the paraxial edge
  // segment connection pass. [This is derived from paraxial segment length & then summed.]
  uint _M_paraxial_seg_saved_sum;
#endif
};


NAMESPACE_CK2_END;
#endif
