#ifndef LIBCK2_PROV_EDGE_H
#define LIBCK2_PROV_EDGE_H

#include <algorithm>
#include <array>
#include <deque>
#include <iterator>

#include "Point.h"
#include "common.h"


NAMESPACE_CK2;


struct ProvEdge {
  // Each ProvEdge regards exactly 2 different "provinces" (sometimes they're meta-provinces like impassable
  // mountains) on either side of it. Since ProvEdges are undirected relationships, we always normalize this ID:ID
  // relation by applying ascending order to the [meta-]province IDs. NOTE: This means that other ProvEdges may
  // "physically" connect up with this one but not be considered part of the same ProvEdge due to having a
  // an incompatible IDPair.

  struct IDPair {
    constexpr IDPair()
      : _M_id({ 0, 0 }) {}

    constexpr IDPair(prov_id_t i1, prov_id_t i2)
      : _M_id({ std::min(i1, i2), std::max(i1, i2) }) {}

    constexpr auto& front() const noexcept { return _M_id.front(); }
    constexpr auto& front()       noexcept { return _M_id.front(); }
    constexpr auto& back()  const noexcept { return _M_id.back(); }
    constexpr auto& back()        noexcept { return _M_id.back(); }

    // The zero-value for a prov_id_t is also the null value, so if either ID is 0, the IDPair is also null.
    constexpr operator bool() const noexcept
    {
      return front() != 0 && back() != 0;
    }

    // Aggregate accessor for when we just need a unique 32-bit ID for the unordered pair.
    constexpr auto unique_id() const noexcept
    {
      return static_cast<uint32_t>( front() ) << 16 | back();
    }

  private:
     std::array<prov_id_t, 2> _M_id;
  };

  using P16 = Point<uint16_t>;

  ProvEdge(IDPair relation, P16 p1, P16 p2)
    : _M_relation(relation), _M_knots( {p1, p2} ) {}

  auto relation() const noexcept { return _M_relation; }

  // STL container-like interface over our knots / control points:

  auto cbegin()  const noexcept { return _M_knots.cbegin(); }
  auto cend()    const noexcept { return _M_knots.cend(); }
  auto begin()   const noexcept { return cbegin(); }
  auto end()     const noexcept { return cend(); }
  auto begin()         noexcept { return _M_knots.begin(); }
  auto end()           noexcept { return _M_knots.end(); }

  auto crbegin() const noexcept { return _M_knots.crbegin(); }
  auto crend()   const noexcept { return _M_knots.crend(); }
  auto rbegin()  const noexcept { return crbegin(); }
  auto rend()    const noexcept { return crend(); }
  auto rbegin()        noexcept { return _M_knots.rbegin(); }
  auto rend()          noexcept { return _M_knots.rend(); }

  auto size()    const noexcept { return _M_knots.size(); }
  auto empty()   const noexcept { return size() == 0; }

  auto& front() const noexcept { return _M_knots.front(); }
  auto& front()       noexcept { return _M_knots.front(); }
  auto& back()  const noexcept { return _M_knots.back(); }
  auto& back()        noexcept { return _M_knots.back(); }

  void append_front(const ProvEdge& e)
  {
    if (front() == e.front())
    {
      // Need to forward-copy the knots of `e` past the front endpoint of this ProvEdge (ignoring the 1st knot,
      // since it'd be a duplicate otherwise).
      std::copy(std::next(e.begin()), e.end(),
                std::front_inserter(_M_knots));
    }
    else
    {
      assert(front() == e.back());
      // Need to copy the knots of `e` in reverse order (starting from the knot prior to its last, as the last
      // would be a duplicate otherwise, and progressing to its beginning) past the front endpoint of this
      // ProvEdge.
      std::reverse_copy(e.begin(), std::prev(e.end()),
                        std::front_inserter(_M_knots));
    }
  }

  void append_back(const ProvEdge& e)
  {
    // MAYBE-TODO: At some point, factor this code with append_front, because it's identical if you parameterize
    // the back/front comparisons with iterators & can somehow or another determine whether a back_inserter or
    // front_inserter is needed.

    if (back() == e.front())
    {
      std::copy(std::next(e.begin()), e.end(),
                std::back_inserter(_M_knots));
    }
    else
    {
      assert(back() == e.back());
      std::reverse_copy(e.begin(), std::prev(e.end()),
                        std::back_inserter(_M_knots));
    }
  }

private:
  // Each ProvEdge has at least 2 2D control points (the endpoints) with an additional, arbitrarily long series
  // of interior control points. These interior points are assumed to be connected in series. In practice, their
  // connections will always be axis-parallel line segments, as we won't waste space describing all the control
  // points which lie upon the same horizontal or vertical line segment by merging them together as we trace.

  // We need to be able to efficiently push new control points past both the ProvEdge's endpoints as we trace
  // it, and yet we cannot ultimately use a std::deque for the Point storage, because for a deque to hold even a
  // single Point, its internal index buffer must be allocated (which is 4KiB on all surveyed 64-bit C++ STL
  // implementations). And that's not all the storage required!

  // Initially, there will be a lot of ProvEdge objects allocated before they're maximally traced & merged
  // together, so it's important that the deque-like implementation of this data structure is space-efficient
  // for very small numbers of Points (yet with the same or better efficiency at double-ended insertion and
  // capacity growth once these structures grow to contain a lot more Points).

  // BUT: premature optimization is the enemy of programmer productivity, so we'll start with std::deque!

  IDPair          _M_relation;
  std::deque<P16> _M_knots; // control points which implicitly define paraxial edge segments between them
};


NAMESPACE_CK2_END;
#endif
