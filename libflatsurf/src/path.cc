/**********************************************************************
 *  This file is part of flatsurf.
 *
 *        Copyright (C) 2020 Julian Rüth
 *
 *  Flatsurf is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Flatsurf is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with flatsurf. If not, see <https://www.gnu.org/licenses/>.
 *********************************************************************/

#include "../flatsurf/path.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <ostream>
#include <unordered_set>

#include "../flatsurf/ccw.hpp"
#include "../flatsurf/chain.hpp"
#include "../flatsurf/fmt.hpp"
#include "../flatsurf/path_iterator.hpp"
#include "../flatsurf/saddle_connection.hpp"
#include "../flatsurf/vector.hpp"
#include "../flatsurf/vertex.hpp"
#include "external/rx-ranges/include/rx/ranges.hpp"
#include "impl/path.impl.hpp"
#include "impl/path_iterator.impl.hpp"
#include "util/assert.ipp"

namespace flatsurf {

template <typename Surface>
Path<Surface>::Path() noexcept :
  self(spimpl::make_impl<ImplementationOf<Path>>()) {}

template <typename Surface>
Path<Surface>::Path(const std::vector<Segment>& path) :
  self(spimpl::make_impl<ImplementationOf<Path>>(path)) {}

template <typename Surface>
Path<Surface>::operator const std::vector<Segment> &() const {
  return self->path;
}

template <typename Surface>
bool Path<Surface>::operator==(const Path& rhs) const {
  return self->path == rhs.self->path;
}

template <typename Surface>
bool Path<Surface>::closed() const {
  if (empty()) return true;
  return ImplementationOf<Path>::connected(*std::rbegin(self->path), *std::begin(self->path));
}

template <typename Surface>
bool Path<Surface>::simple() const {
  using std::begin, std::end;
  std::unordered_set<Segment> segments(begin(self->path), end(self->path));
  return segments.size() == self->path.size();
}

template <typename Surface>
Path<Surface> Path<Surface>::reversed() const {
  return self->path | rx::transform([&](const auto& connection) { return -connection; }) | rx::reverse() | rx::to_vector();
}

template <typename Surface>
bool Path<Surface>::empty() const {
  return self->path.empty();
}

template <typename Surface>
size_t Path<Surface>::size() const {
  return self->path.size();
}

template <typename Surface>
void Path<Surface>::push_front(const Segment& segment) {
  ASSERT(empty() || ImplementationOf<Path>::connected(segment, *begin()), "Path must be connected but " << segment << " does not precede " << *begin() << " either because they are connected to different vertices or because the turn from " << -segment << " to " << *begin() << " is not turning clockwise in the range (0, 2π]");
  self->path.insert(std::begin(self->path), segment);
}

template <typename Surface>
void Path<Surface>::push_back(const Segment& segment) {
  ASSERT(empty() || ImplementationOf<Path>::connected(*self->path.rbegin(), segment), "Path must be connected but " << *self->path.rbegin() << " does not precede " << segment << " either because they are connected to different vertices or because the turn from " << -*self->path.rbegin() << " to " << segment << " is not turning clockwise in the range (0, 2π]");
  self->path.push_back(segment);
}

template <typename Surface>
void Path<Surface>::splice(const PathIterator<Surface>& pos, Path& other) {
  ASSERT_ARGUMENT(this != &other, "Cannot splice path into itself");

  const auto at = pos == end() ? std::end(self->path) : pos.self->position;
  self->path.insert(at, std::begin(other.self->path), std::end(other.self->path));

  ASSERTIONS([&]() {
    for (auto segment = std::begin(self->path); segment != std::end(self->path); segment++) {
      ASSERT(segment + 1 == std::end(self->path) || ImplementationOf<Path>::connected(*segment, *(segment + 1)), "Path must be connected but " << *segment << " does not precede " << *(segment + 1) << " either because they are connected to different vertices or because the turn from " << -*segment << " to " << *(segment + 1) << " is not turning clockwise in the range (0, 2π]");
    }
    return true;
  });

  other.self->path.clear();
}

template <typename Surface>
void Path<Surface>::splice(const PathIterator<Surface>& pos, Path&& other) {
  // This could be done more efficiently if it shows up in the profiler.
  splice(pos, other);
}

template <typename Surface>
typename Surface::Coordinate Path<Surface>::area() const {
  CHECK_ARGUMENT(closed(), "Area can only be computed for closed paths but " << *this << " is not closed.");
  return Vector<T>::area(*this | rx::transform([](const auto& connection) { return connection.vector(); }) | rx::to_vector());
}

template <typename Surface>
PathIterator<Surface> Path<Surface>::begin() const {
  return PathIterator<Surface>(PrivateConstructor{}, this, std::begin(self->path));
}

template <typename Surface>
PathIterator<Surface> Path<Surface>::end() const {
  return PathIterator<Surface>(PrivateConstructor{}, this, std::end(self->path));
}

template <typename Surface>
std::ostream& operator<<(std::ostream& os, const Path<Surface>& path) {
  return os << fmt::format("[{}]", fmt::join(path.self->path, " → "));
}

template <typename Surface>
ImplementationOf<Path<Surface>>::ImplementationOf() :
  path() {}

template <typename Surface>
ImplementationOf<Path<Surface>>::ImplementationOf(const std::vector<Segment>& path) {
  for (auto segment = begin(path); segment != end(path); segment++) {
    ASSERT(segment + 1 == end(path) || connected(*segment, *(segment + 1)), "Path must be connected but " << *segment << " does not precede " << *(segment + 1) << " either because they are connected to different vertices or because the turn from " << -*segment << " to " << *(segment + 1) << " is not turning clockwise in the range (0, 2π]");
    this->path.push_back(*segment);
  }
}

template <typename Surface>
bool ImplementationOf<Path<Surface>>::connected(const Segment& a, const Segment& to) {
  const auto& surface = a.surface();

  const auto from = -a;

  const auto ccw = [](const Chain<Surface>& lhs, const Chain<Surface>& rhs) {
    // In typical flow decompositions, running ccw on approximations first
    // does not seem to help the runtime of this method:
    const auto approximate = static_cast<const Vector<exactreal::Arb>&>(lhs).ccw(static_cast<const Vector<exactreal::Arb>&>(rhs));
    if (approximate)
      return *approximate;
    return static_cast<const Vector<T>&>(lhs).ccw(static_cast<const Vector<T>&>(rhs));
  };

  if (Vertex::source(from.source(), surface) != Vertex::source(to.source(), surface))
    return false;

  if (from.source() == to.source()) {
    // If from and to are in the same sector, we essentially check whether
    // we have to turn clockwise between them.
    if (ccw(from, to) == CCW::CLOCKWISE) {
      return true;
    } else {
      return surface.angle(Vertex::source(from.source(), surface)) == 1;
    }
  } else {
    // If from and to are not in the same sector, we go through the sectors until we hit to.source().
    // We keep track how far sector is turned from from.vector() in multiples of π.
    int turn = 0;
    for (auto sector = surface.previousAtVertex(from.source()); turn != 2; sector = surface.previousAtVertex(sector)) {
      const auto chain = Chain(surface, sector);
      if (turn == 0) {
        if (ccw(from, chain) != CCW::CLOCKWISE) {
          turn = 1;
        }
      } else if (turn == 1) {
        if (ccw(from, chain) == CCW::CLOCKWISE) {
          turn = 2;
        }
      }

      if (sector == to.source()) {
        if (turn < 2) {
          // The entire sector is at most 2π from the "from" vector so "to" must be within a 2π turn.
          return true;
        } else {
          // Parts of the sector are more than a 2π turn from the "from" vector.
          return ccw(from, to) != CCW::CLOCKWISE;
        }
      }
    }
    return false;
  }
}
}  // namespace flatsurf

// Instantiations of templates so implementations are generated for the linker
#include "util/instantiate.ipp"

LIBFLATSURF_INSTANTIATE_MANY_WRAPPED((LIBFLATSURF_INSTANTIATE_WITH_IMPLEMENTATION), Path, LIBFLATSURF_FLAT_TRIANGULATION_TYPES)
