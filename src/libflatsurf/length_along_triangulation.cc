/**********************************************************************
 *  This file is part of flatsurf.
 *
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

#include <exact-real/yap/arb.hpp>
#include <optional>
#include <gmpxx.h>
#include <boost/lexical_cast.hpp>

#include "flatsurf/length_along_triangulation.hpp"
#include "flatsurf/vector_along_triangulation.hpp"
#include "flatsurf/vector.hpp"
#include "flatsurf/half_edge.hpp"
#include "flatsurf/half_edge_map.hpp"
#include "flatsurf/flat_triangulation.hpp"

// TODO: Many of the assertions here should be argument checks.

using std::optional;
using boost::lexical_cast;
using exactreal::Arb;
using exactreal::ARB_PRECISION_FAST;

namespace flatsurf {
namespace {
enum CLASSIFICATION {
  BOTH_ARE_NIL,
  LEFT_IS_NIL,
  RIGHT_IS_NIL,
  NONE_IS_NIL,
};
}

template <typename T>
class LengthAlongTriangulation<T>::Implementation {
 public:
  using Coefficient = std::conditional_t<std::is_same_v<long long, T>, long long, mpz_class>;

  static void updateAfterFlip(HalfEdgeMap<Coefficient>& map, HalfEdge halfEdge, const FlatTriangulationCombinatorial& parent) {
    map.set(-parent.nextAtVertex(halfEdge), map.get(halfEdge));
    map.set(-parent.nextInFace(parent.nextInFace(halfEdge)), map.get(halfEdge));
    map.set(halfEdge, 0);
  }

  Implementation() : parent(nullptr), horizontal(nullptr), coefficients(), approximation() {}

  Implementation(Surface const * parent, Vector<T> const * horizontal, const HalfEdge e) : parent(parent), horizontal(horizontal), coefficients(HalfEdgeMap<Coefficient>(this->parent, updateAfterFlip)), approximation() {
    coefficients->set(e, 1);
    approximation = static_cast<Vector<Arb>>(parent->fromEdge(e)) * static_cast<Vector<Arb>>(*horizontal);

    assert(static_cast<T>(*this) >= 0 && "Lenghts must not be negative");
  }

  operator T() const noexcept {
    if (!coefficients)
      return T();
    T ret;
    coefficients->apply([&](const HalfEdge e, const Coefficient& c) {
      ret += c * (parent->fromEdge(e) * *horizontal);
    });
    return ret;
  }

  CLASSIFICATION classify(const Implementation& rhs) const {
    if (!coefficients && !rhs.coefficients)
      return BOTH_ARE_NIL;
    if (!coefficients)
      return LEFT_IS_NIL;
    if (!rhs.coefficients)
      return RIGHT_IS_NIL;

    assert(parent == rhs.parent && "Lengths must be in the same triangulation.");
    assert(horizontal == rhs.horizontal && "Lengths must be relative to the same horizontal vector.");

    return NONE_IS_NIL;
  }

  Surface const * parent;
  Vector<T> const * horizontal;
  std::optional<HalfEdgeMap<Coefficient>> coefficients;
  Arb approximation;
};

template <typename T>
LengthAlongTriangulation<T>::LengthAlongTriangulation() : impl(spimpl::make_impl<Implementation>()) {}

template <typename T>
LengthAlongTriangulation<T>::LengthAlongTriangulation(Surface const * parent, Vector<T> const * horizontal, const HalfEdge e) : impl(spimpl::make_impl<Implementation>(parent, horizontal, e)) {}

template <typename T>
bool LengthAlongTriangulation<T>::operator==(const LengthAlongTriangulation& rhs) const {
  auto maybe = impl->approximation == rhs.impl->approximation;
  if (maybe)
    return *maybe;
  return static_cast<T>(*impl) == static_cast<T>(*rhs.impl);
}

template <typename T>
bool LengthAlongTriangulation<T>::operator<(const LengthAlongTriangulation& rhs) const {
  auto maybe = impl->approximation < rhs.impl->approximation;
  if (maybe)
    return *maybe;
  return static_cast<T>(*impl) < static_cast<T>(*rhs.impl);
}

template <typename T>
LengthAlongTriangulation<T>& LengthAlongTriangulation<T>::operator+=(const LengthAlongTriangulation& rhs) {
  switch(impl->classify(*rhs.impl)) {
    case BOTH_ARE_NIL:
    case RIGHT_IS_NIL:
      break;
    case LEFT_IS_NIL:
      impl = rhs.impl;
      break;
    case NONE_IS_NIL:
      impl->approximation += rhs.impl->approximation (ARB_PRECISION_FAST);
      rhs.impl->coefficients->apply([&](const HalfEdge e, const typename Implementation::Coefficient& c) {
        impl->coefficients->set(e, impl->coefficients->get(e) + c);
      });
      break;
  }
  return *this;
}

template <typename T>
LengthAlongTriangulation<T>& LengthAlongTriangulation<T>::operator-=(const LengthAlongTriangulation& rhs) {
  switch(impl->classify(*rhs.impl)) {
    case BOTH_ARE_NIL:
    case RIGHT_IS_NIL:
      break;
    case LEFT_IS_NIL:
      throw std::logic_error("Can not subtract non-zero length from zero length.");
    case NONE_IS_NIL:
      assert(rhs <= *this && "Can not subtract length from smaller length.");
      impl->approximation -= rhs.impl->approximation (ARB_PRECISION_FAST);
      rhs.impl->coefficients->apply([&](const HalfEdge e, const typename Implementation::Coefficient& c) {
        impl->coefficients->set(e, impl->coefficients->get(e) - c);
      });
      assert(*this >= LengthAlongTriangulation() && "Lengths must not be negative");
      break;
  }
  return *this;
}

template <typename T>
LengthAlongTriangulation<T>& LengthAlongTriangulation<T>::operator*=(const Quotient& rhs) {
  if (impl->coefficients) {
    impl->coefficients->apply([&](const HalfEdge e, const typename Implementation::Coefficient& c) {
      if constexpr (std::is_same_v<long long, T>) {
        assert(rhs * mpz_class(lexical_cast<std::string>(c)) <= mpz_class(lexical_cast<std::string>(LONG_LONG_MAX)) && "Multiplication overflow");
        impl->coefficients->set(e, c * lexical_cast<long long>(lexical_cast<std::string>(rhs)));
      } else {
        impl->coefficients->set(e, c * rhs);
      }
    });
    impl->approximation *= Arb(rhs) (ARB_PRECISION_FAST);
  }
  return *this;
}

template <typename T>
LengthAlongTriangulation<T>::operator bool() const noexcept {
  if (!impl->coefficients)
    return false;
  auto maybe = impl->approximation == Arb();
  if (maybe)
    return !*maybe;
  return static_cast<bool>(static_cast<T>(*impl));
}

template <typename T>
typename LengthAlongTriangulation<T>::Quotient LengthAlongTriangulation<T>::operator/(const LengthAlongTriangulation& rhs) {
  assert(rhs && "Division by zero length");
  if (!*this) {
    return mpz_class(0);
  }

  mpz_class quo = static_cast<Arb>((impl->approximation / rhs.impl->approximation) (ARB_PRECISION_FAST)).floor();

  while (rhs * quo < *this) {
    quo++;
  }
  while (rhs * quo > *this) {
    quo--;
  }
  return quo;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const LengthAlongTriangulation<T>& self) {
  if (!self)
    return os << 0;
  else
    throw std::logic_error("not implemented: operator<<(LengthAlongTriangulation)");
}
}

// Instantiations of templates so implementations are generated for the linker
#include <e-antic/renfxx.h>
#include <exact-real/element.hpp>
#include <exact-real/integer_ring_traits.hpp>
#include <exact-real/number_field_traits.hpp>
#include <exact-real/rational_field_traits.hpp>

namespace flatsurf {
using eantic::renf_elem_class;
using exactreal::Element;
using exactreal::IntegerRingTraits;
using exactreal::NumberFieldTraits;
using exactreal::RationalFieldTraits;

template class LengthAlongTriangulation<long long>;
template std::ostream& operator<<(std::ostream&, const LengthAlongTriangulation<long long>&);
template class LengthAlongTriangulation<renf_elem_class>;
template std::ostream& operator<<(std::ostream&, const LengthAlongTriangulation<renf_elem_class>&);
template class LengthAlongTriangulation<Element<IntegerRingTraits>>;
template std::ostream& operator<<(std::ostream&, const LengthAlongTriangulation<Element<IntegerRingTraits>>&);
template class LengthAlongTriangulation<Element<RationalFieldTraits>>;
template std::ostream& operator<<(std::ostream&, const LengthAlongTriangulation<Element<RationalFieldTraits>>&);
template class LengthAlongTriangulation<Element<NumberFieldTraits>>;
template std::ostream& operator<<(std::ostream&, const LengthAlongTriangulation<Element<NumberFieldTraits>>&);
}
