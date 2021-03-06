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

#include "../flatsurf/half_edge_set.hpp"

#include <fmt/format.h>

#include <ostream>

#include "../flatsurf/flat_triangulation_combinatorial.hpp"
#include "../flatsurf/fmt.hpp"
#include "../flatsurf/half_edge.hpp"
#include "../flatsurf/half_edge_set_iterator.hpp"
#include "impl/half_edge_set.impl.hpp"
#include "util/assert.ipp"

namespace flatsurf {

HalfEdgeSet::HalfEdgeSet() noexcept :
  self(spimpl::make_impl<ImplementationOf<HalfEdgeSet>>()) {}

HalfEdgeSet::HalfEdgeSet(const std::vector<HalfEdge>& items) :
  self(spimpl::make_impl<ImplementationOf<HalfEdgeSet>>(items)) {}

bool HalfEdgeSet::contains(HalfEdge e) const {
  ASSERT(e != HalfEdge(), "cannot check containment of invalid HalfEdge(0)");
  return self->contains(e);
}

void HalfEdgeSet::insert(HalfEdge e) {
  ASSERT(e != HalfEdge(), "cannot insert invalid HalfEdge(0)");
  self->insert(e);
}

bool HalfEdgeSet::operator==(const HalfEdgeSet& rhs) const {
  return *self == *rhs.self;
}

bool HalfEdgeSet::disjoint(const HalfEdgeSet& rhs) const {
  return self->disjoint(*rhs.self);
}

bool HalfEdgeSet::empty() const {
  return self->empty();
}

size_t HalfEdgeSet::size() const {
  return self->size();
}

void HalfEdgeSet::erase(HalfEdge e) {
  ASSERT(e != HalfEdge(), "cannot remove invalid HalfEdge(0)");
  return self->erase(e);
}

HalfEdgeSetIterator begin(const HalfEdgeSet& self) { return self.begin(); }

HalfEdgeSetIterator end(const HalfEdgeSet& self) { return self.end(); }

std::ostream& operator<<(std::ostream& os, const HalfEdgeSet& self) {
  return os << fmt::format("{{{}}}", fmt::join(begin(self), end(self), ", "));
}

}  // namespace flatsurf
