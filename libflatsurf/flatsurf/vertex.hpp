/**********************************************************************
 *  This file is part of flatsurf.
 *
 *        Copyright (C) 2019-2020 Julian Rüth
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

#ifndef LIBFLATSURF_VERTEX_HPP
#define LIBFLATSURF_VERTEX_HPP

#include <boost/operators.hpp>

#include "external/spimpl/spimpl.h"

#include "copyable.hpp"
#include "forward.hpp"
#include "serializable.hpp"

namespace flatsurf {
class Vertex : Serializable<Vertex>,
               boost::equality_comparable<Vertex> {
  // Vertex cannot be created directly; use source() and target() instead.
  template <typename ...Args> Vertex(PrivateConstructor, Args&&...);

 public:
  static const Vertex& source(const HalfEdge &, const FlatTriangulationCombinatorial &);
  static const Vertex& target(const HalfEdge &, const FlatTriangulationCombinatorial &);

  bool operator==(const Vertex &) const;

  friend std::ostream &operator<<(std::ostream &, const Vertex &);

 private:
  using Implementation = ImplementationOf<Vertex>;
  Copyable<Implementation> impl;
  friend Implementation;
  friend std::hash<Vertex>;
};
}  // namespace flatsurf

// Makes Vertex hashable.
template <>
struct std::hash<flatsurf::Vertex> {
  size_t operator()(const flatsurf::Vertex &) const noexcept;
};

#endif
