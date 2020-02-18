/**********************************************************************
 *  This file is part of flatsurf.
 *
 *        Copyright (C) 2020 Vincent Delecroix
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

#ifndef LIBFLATSURF_TEST_GENERATORS_SURFACE_GENERATORS_HPP
#define LIBFLATSURF_TEST_GENERATORS_SURFACE_GENERATORS_HPP

#include <memory>

#include <gmpxx.h>
#include <e-antic/renf_elem.h>

#include "../external/catch2/single_include/catch2/catch.hpp"

#include "../../flatsurf/forward.hpp"

#include "../surfaces.hpp"

namespace flatsurf::test {

template <typename T>
class SurfaceGenerator : public Catch::Generators::IGenerator<std::shared_ptr<FlatTriangulation<T>>> {
  enum class Surface {
    SQUARE,
    L,
    _125,
    _1221,
    HEXAGON,
    HEPTAGON_L,
    GOLDEN_L,
    LAST = GOLDEN_L,
  };

  Surface current;
  std::shared_ptr<FlatTriangulation<T>> value;

  using R2 = Vector<T>;

  static constexpr bool hasFractions = std::is_same_v<T, mpq_class> || std::is_same_v<T, exactreal::Element<exactreal::RationalField>> || std::is_same_v<T, exactreal::Element<exactreal::NumberField>>;
  static constexpr bool hasNumberFieldElements = std::is_same_v<T, eantic::renf_elem_class> || std::is_same_v<T, exactreal::Element<exactreal::NumberField>>;

  static std::shared_ptr<FlatTriangulation<T>> make(Surface surface) {
    switch(surface) {
      case Surface::SQUARE:
        return makeSquare<R2>();
      case Surface::L:
        return makeL<R2>();
      case Surface::_125:
        if constexpr (hasFractions && hasNumberFieldElements)
          return make125<R2>();
        else return nullptr;
      case Surface::_1221:
        if constexpr (hasFractions && hasNumberFieldElements)
          return make1221<R2>();
        else return nullptr;
      case Surface::HEXAGON:
        if constexpr (hasFractions && hasNumberFieldElements)
          return makeHexagon<R2>();
        else return nullptr;
      case Surface::HEPTAGON_L:
        if constexpr (hasFractions && hasNumberFieldElements)
          return makeHeptagonL<R2>();
        else return nullptr;
      case Surface::GOLDEN_L:
        if constexpr (hasNumberFieldElements)
          return makeGoldenL<R2>();
        else return nullptr;
      default:
        throw std::logic_error("unknown Surface type");
    }
  }

public:
  SurfaceGenerator() :
    current(Surface::SQUARE),
    value(make(current)) {}

  const std::shared_ptr<FlatTriangulation<T>>& get() const override {
    return value;
  }

  bool next() override {
    if (current == Surface::LAST) return false;

    current = static_cast<Surface>(static_cast<int>(current) + 1);

    value = make(current);

    if (value == nullptr)
      return next();

    return true;
  }
};

template <typename T>
Catch::Generators::GeneratorWrapper<std::shared_ptr<FlatTriangulation<T>>> makeSurface() {
  return Catch::Generators::GeneratorWrapper<std::shared_ptr<FlatTriangulation<T>>>(std::unique_ptr<Catch::Generators::IGenerator<std::shared_ptr<FlatTriangulation<T>>>>(new SurfaceGenerator<T>()));
}

}

#endif
