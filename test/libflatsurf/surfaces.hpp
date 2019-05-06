/**********************************************************************
 *  This file is part of flatsurf.
 *
 *        Copyright (C) 2019 Vincent Delecroix
 *        Copyright (C) 2019 Julian Rüth
 *
 *  Flatsurf is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
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

#include <vector>

#include <exact-real/integer_ring_traits.hpp>
#include <flatsurf/flat_triangulation.hpp>
#include <flatsurf/vector_eantic.hpp>
#include <flatsurf/vector_exactreal.hpp>
#include <flatsurf/vector_longlong.hpp>

using namespace flatsurf;
using std::vector;
using eantic::renf_class;

namespace {
template <typename R2>
auto makeSquare() {
  auto vectors = vector{R2(1, 0), R2(0, 1), R2(1, 1)};
  auto vertices = vector<vector<int>>{{1, 3, 2, -1, -3, -2}};
  return FlatTriangulation(vertices, vectors);
}

template <typename R2>
auto makeHexagon() {
  auto K = renf_class("x^2 - 3", "x", "1.73 +/- 0.1");
  auto x = K.gen();
  auto vectors = vector{R2(2, 0), R2(1, x), R2(3, x), R2(1, -x), R2(4, 0), R2(3, x)};
  auto vertices = vector<vector<int>>({{1, 3, -4, -5, -3, -2}, {2, -1, -6, 4, 5, 6}});
  return FlatTriangulation(vertices, vectors);
}
}
