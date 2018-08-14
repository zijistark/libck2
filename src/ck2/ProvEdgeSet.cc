#include "ProvEdgeSet.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <vector>

#include "Point.h"
#include "ProvEdge.h"
#include "ProvMap.h"

using namespace std;


NAMESPACE_CK2;


ProvEdgeSet::ProvEdgeSet(const ProvMap& pm)
  : _M_grid(pm.data())
  , _M_width(pm.width())
  , _M_height(pm.height())
#ifdef DEBUG
  , _M_paraxial_seg_count(0)
  , _M_paraxial_seg_max((pm.width() + 1)/2 + (pm.height() + 1)/2)
  , _M_paraxial_seg_len_sum(0)
  , _M_paraxial_seg_saved_sum(0)
#endif
{
  assert(pm.width() <= numeric_limits<decltype(_M_width)>::max());
  assert(pm.height() <= numeric_limits<decltype(_M_height)>::max());
  assert(_M_width >= 3 && _M_height >= 3);

  uint max_x = _M_width - 1;
  uint max_y = _M_height - 1;

  EndpointMap endpoint_map; // Will map ProvEdge endpoints (i.e., its keys) to each other (i.e., is a multimap).

  std::vector<Segment> vertical_seg(_M_width); // vertical segments (fixed x-coord)
  Segment horizontal_seg; // horizontal segment (fixed y-coord) -- in scan order, so only 1 is required

  auto p_pix = _M_grid;

  /* handle top row */

  for (uint x = 0; x < max_x; ++x, ++p_pix) {
    // handle top-left corner + top-center row
    try_vertical_edge(x, 0, p_pix, vertical_seg[x], endpoint_map);
    try_horizontal_edge(x, 0, p_pix, horizontal_seg, endpoint_map);
  }

  // handle top-right corner
  if (try_horizontal_edge(max_x, 0, p_pix++, horizontal_seg, endpoint_map))
    finish_segment(Direction::Horizontal, 0, horizontal_seg, endpoint_map);

  /* handle middle rows */

  for (uint y = 1; y < max_y; ++y)
  {
    for (uint x = 0; x < max_x; ++x, ++p_pix)
    {
      // handle middle-left pixel + middle-center row
      try_vertical_edge(x, y, p_pix, vertical_seg[x], endpoint_map);
      try_horizontal_edge(x, y, p_pix, horizontal_seg, endpoint_map);
    }

    // handle middle-right pixel
    if (try_horizontal_edge(max_x, y, p_pix++, horizontal_seg, endpoint_map))
      finish_segment(Direction::Horizontal, y, horizontal_seg, endpoint_map);
  }

  /* handle bottom row */

  for (uint x = 0; x < max_x; ++x, ++p_pix)
    if (try_vertical_edge(x, max_y, p_pix, vertical_seg[x], endpoint_map))
      finish_segment(Direction::Vertical, x, horizontal_seg, endpoint_map);

  // ... and the bottom-right pixel actually cannot produce any new paraxial edges, so we're done with paraxial
  // segment scanning.

  if (_M_edges.empty())
    return; // Within this context, it's valid to have not produced any edges at all, but we should quit if so.

  // Now, it's time to join our ProvEdge objects (created by finish_segment(...)) together by their endpoints
  // until they're maximally traced.

  for (uint edge_idx = 0; edge_idx < _M_edges.size(); ++edge_idx)
  {
    if (!_M_edges[edge_idx])
      continue; // Skip edge table entries which have been released (due to tracing of edges consuming them).

    // Maximally trace this edge from both of its endpoints:
    trace_edge(edge_idx, endpoint_map);
  }

  // Now, compact our edge vector to only include contiguous non-null edge pointers:

  uint wr_idx = 0;

  for (uint rd_idx = 0; rd_idx < _M_edges.size(); ++rd_idx)
  {
    if (_M_edges[rd_idx])
    {
      if (rd_idx != wr_idx)
        _M_edges[wr_idx] = std::move(_M_edges[rd_idx]);

      ++wr_idx;
    }
  }

  assert(wr_idx > 0);
  _M_edges.resize(wr_idx);
  _M_edges.shrink_to_fit();
}


void ProvEdgeSet::finish_segment(Direction d, coord_t fixed_coord, const Segment& seg, EndpointMap& map)
{
#ifdef DEBUG
  auto len = seg.end - seg.start;
  _M_paraxial_seg_saved_sum += len - 1;
  _M_paraxial_seg_len_sum += len;
  ++_M_paraxial_seg_count;
#endif

  // Push a new ProvEdge object on to our vector of std::unique_ptr to ProvEdge. Convert Segment to 2 [end]Points.

  const auto& edge = _M_edges.emplace_back(
    std::make_unique<ProvEdge>(
      ProvEdge::IDPair(seg.relation.first, seg.relation.second),
      (d == Direction::Vertical) ? LPoint(fixed_coord, seg.start) : LPoint(seg.start, fixed_coord),
      (d == Direction::Vertical) ? LPoint(fixed_coord, seg.end)   : LPoint(seg.end,   fixed_coord)
    )
  );

  uint idx = _M_edges.size() - 1; // And we'll refer to it by this new index hereafter.

  // Now, we insert the ProvEdge's index (i.e., the value) into our multimap twice, once by each endpoint (i.e.,
  // the key):

  map.emplace(edge->front(), idx);
  map.emplace(edge->back(), idx);
}


void ProvEdgeSet::trace_edge_end(EdgeEnd which_end, uint edge_idx, EndpointMap& map)
{
  assert( _M_edges[edge_idx] );
  auto& edge = *_M_edges[edge_idx];
  assert( edge.size() >= 2 );

  while (true)
  {
    auto my_it = map.end(), other_it = map.end();
    auto range = map.equal_range( (which_end == EdgeEnd::Front) ? edge.front() : edge.back() );

    // Among the max. of 4 possible endpoints returned in this iterator range, find the one which corresponds to
    // the current edge, and find the one (if any) that can be joined with / consumed by the current edge:

    for (auto it = range.first; it != range.second; ++it)
    {
      if (it->second == edge_idx)
        my_it = it;
      else
      {
        assert( _M_edges[ it->second ] );

        if (_M_edges[ it->second ]->relation() == edge.relation())
          other_it = it;

        // There can only be a maximum of 1 such edge relation match at this endpoint. Casual proof:

        // A) Our definition of edges requires a constant, undirected entity relation on either side of them.

        // B) We cannot join an edge with itself.

        // C) Any edge segment at this endpoint which shares the same fixed-axis as the current edge's end
        // segment cannot have the same edge relation, because it would've already been joined in the paraxial
        // segment scan.

        // D) The only other 2 orientations of edge segments which end at this point are also subject to (c) so
        // cannot have the same edge relation as each other (and are perpendicular to any segment whose relation
        // they might match here).

        // E) Ergo, only 1 of the 4 possible edge segments which meet at this point can share the same edge
        // relation as `edge` (and the segment to which it belongs, that of the adjoining edge, runs
        // perpendicular to this side of `edge`).
      }
    }

    assert( my_it != map.end() );
    assert( my_it != other_it );

    if (other_it == map.end())
      return; // No further tracing possible in this direction, so we're done here.

    auto other_edge_idx = other_it->second;
    assert( _M_edges[other_edge_idx] );
    auto& other_edge = *_M_edges[other_edge_idx];
    assert( other_edge.size() >= 2 );

    // The old endpoint entries from where we will join edges in the map are now obsolete, so remove them:
    map.erase(my_it);
    map.erase(other_it);

    if (which_end == EdgeEnd::Front)
      edge.append_front(other_edge);
    else
      edge.append_back(other_edge);

    auto new_endpoint = (which_end == EdgeEnd::Front) ? edge.front() : edge.back();

    // The new endpoint on this side of `edge` is the same as the opposite endpoint of `other_edge` now, so find
    // that preexisting endpoint & update its value to point to our own edge index.

    auto new_endpoint_it = map.end();
    auto next_range = map.equal_range(new_endpoint);

    for (auto it = next_range.first; it != next_range.second; ++it)
    {
      if (it->second == other_edge_idx)
      {
        new_endpoint_it = it;
        break;
      }
    }

    assert( new_endpoint_it != map.end() ); // thing better be there already!
    new_endpoint_it->second = edge_idx; // now the entry points at the edge which ate other_edge instead.

    // We're 100% done with `other_edge` now, so release it (i.e., destroy & free it while nulling its
    // owning std::unique_ptr).
    _M_edges[other_edge_idx].reset(nullptr);
  }
}


NAMESPACE_CK2_END;
