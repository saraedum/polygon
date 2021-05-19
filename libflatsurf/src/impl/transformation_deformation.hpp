/**********************************************************************
 *  This file is part of flatsurf.
 *
 *        Copyright (C) 2021 Julian Rüth
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

#ifndef LIBFLATSURF_TRANSFORMATION_DEFORMATION_IMPL_HPP
#define LIBFLATSURF_TRANSFORMATION_DEFORMATION_IMPL_HPP

#include "../../flatsurf/half_edge_map.hpp"
#include "./deformation.impl.hpp"

// TODO: Delete

namespace flatsurf {

template <typename Surface>
class TransformationDeformation : ImplementationOf<Deformation<Surface>> {
 public:
  TransformationDeformation(const Surface& source, const Surface& target, HalfEdgeMap<HalfEdge>&&);

  static Deformation<Surface> make(const Surface& source, const Surface& target, HalfEdgeMap<HalfEdge>&&);

  HalfEdgeMap<HalfEdge> isomorphism;
};

}  // namespace flatsurf

#endif
