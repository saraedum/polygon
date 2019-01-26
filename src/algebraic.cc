/**********************************************************************
 *  This file is part of Polygon.
 *
 *        Copyright (C) 2018 Alex Eskin
 *        Copyright (C) 2019 Julian Rüth
 *
 *  Polygon is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Polygon is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Polygon. If not, see <https://www.gnu.org/licenses/>.
 *********************************************************************/

#include <complex>
#include <ostream>
#include <vector>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>

#include "libpolygon/algebraic.h"
#include "libpolygon/defs.h"
#include "libpolygon/number_field.h"
#include "libpolygon/poly.h"
#include "libpolygon/vector.h"

using namespace polygon;
using std::complex;
using std::cout;
using std::endl;
using std::max;
using std::ostream;
using std::vector;
using std::pow;
using boost::numeric_cast;

namespace polygon {
// gcd_extended: a and b inputs, x and y outputs.
template <class T>
void gcd_extended(poly<T> &x, poly<T> &y, const poly<T> &a, const poly<T> &b) {
  poly<T> u, v, quot, rem;
  divide(quot, rem, a, b);
  if (rem == poly<T>())  // i.e. b divides a
  {
    x = poly<T>();  // x=0
    vector<T> ycoeff(1, 1 / b.leading_coefficient());
    y = poly<T>(ycoeff);  // y=1
    return;
  } else {
    gcd_extended(u, v, b, rem);
    x = v;
    y = u - v * quot;
    x.reduce();
    y.reduce();
    return;
  }
}

template <class T>
poly<T>::poly() {
  vector<T> temp(1, T(0));
  coefficients = temp;
}

template <class T>
poly<T>::poly(vector<T> c) : coefficients(c) {
  reduce();
}

template <class T>
poly<T>::poly(T c[], int deg) {
	assert(deg >= -1);
  vector<T> v;
  for (int i = 0; i <= deg; i++) v.push_back(c[i]);
  coefficients = v;
}

template <class T>
T poly<T>::leading_coefficient() const {
  return *(coefficients.end() - 1);
}

template <class T>
void poly<T>::reduce() {
  while (coefficients.back() == T(0) && coefficients.size() > 1)
    coefficients.erase(coefficients.end() - 1);
}

template <class T>
int poly<T>::degree() const {
  //  reduce();
  return numeric_cast<int>(coefficients.size()) - 1;
}

template <class T>
void poly<T>::fill_back(int n) {
  while (coefficients.size() <= numeric_cast<unsigned int>(n)) coefficients.push_back(T(0));
}

template <class T>
void poly<T>::print() {
  cout << *this << "\n";
}

template <class T>
poly<T> &poly<T>::operator+=(const poly<T> &p) {
	if (p.degree() < 0) {
		return *this;
	}
  int m = max(degree(), p.degree());
  fill_back(m);
  for (size_t i = 0; i <= numeric_cast<size_t>(p.degree()); i++)
	  coefficients[i] += p.coefficients[i];
  reduce();
  return *this;
}

template <class T>
poly<T> &poly<T>::operator-=(const poly<T> &p) {
	if (p.degree() < 0){
		return *this;
	}
  int m = max(degree(), p.degree());
  fill_back(m);
  for (size_t i = 0; i <= numeric_cast<size_t>(p.degree()); i++)
		coefficients[i] -= p.coefficients[i];
  reduce();
  return *this;
}

// fix this later
template <class T>
poly<T> operator*(poly<T> p, const poly<T> &q) {
	if (p.degree() < 0 || q.degree() < 0){
		return poly<T>();
	}
  vector<T> c;
  T temp;
  size_t m = numeric_cast<size_t>(p.degree());
	size_t n = numeric_cast<size_t>(q.degree());
  for (size_t i = 0; i <= m + n; i++) {
    temp = 0;
    for (size_t k = n > i ? 0 : i - n; k <= m && k <= i; k++)
      temp += p.coefficients[k] * q.coefficients[i - k];
    c.push_back(temp);
  }
  p = poly<T>(c);
  p.reduce();
  return p;
}

template <class T>
poly<T> operator+(poly<T> p, const poly<T> &q) {
  p += q;
  p.reduce();
  return p;
}

template <class T>
poly<T> operator-(poly<T> p, const poly<T> &q) {
  p -= q;
  p.reduce();
  return p;
}

template <class T>
poly<T> &poly<T>::operator*=(T r) {
  for (typename vector<T>::iterator i = coefficients.begin();
       i != coefficients.end(); ++i)
    *i = r * (*i);
  reduce();
  return *this;
}

template <class T>
poly<T> operator*(T r, poly<T> p) {
  //  poly<T> q = p;
  p *= r;
  p.reduce();
  return p;
}

template <class T>
bool operator==(const poly<T> &p, const poly<T> &q) {
  if (p.coefficients == q.coefficients)
    return true;
  else
    return false;
}

template <class T>
bool operator!=(const poly<T> &p, const poly<T> &q) {
  return (!(p == q));
}

template <class T>
algebraic<T> poly<T>::operator()(const algebraic<T> &a) {
  algebraic<T> b(a.field());
	if (degree() < 0){
		return b;
	}
  for (size_t i = 0; i <= numeric_cast<size_t>(degree()); i++)
		b += coefficients[i] * a.pow(i);
  return b;
}

inline bool is_unit(const poly<int64_t> &p) {
  long n = p.leading_coefficient();
  if (n == 1 || n == -1) {
    return true;
  }
  return false;
}

inline bool is_unit(const poly<bigrat> &p) {
  bigrat q = p.leading_coefficient();
  if (q != bigrat(0)) {
    return true;
  }
  return false;
}

// f and g are inputs
template <class T>
void divide(poly<T> &quotient, poly<T> &remainder, const poly<T> &f,
            const poly<T> &g) {
  int m = f.degree();
  int n = g.degree();
  if (!is_unit(g)) {
    ERR_RET("Division by non-unit");
  }
  if (m < n) {
    remainder = f;
    quotient = poly<T>();
    return;
  }

  poly<T> h = f;
	size_t out_degree = numeric_cast<size_t>(m - n);
  vector<T> c(out_degree + 1);

  for (size_t i = 0; i <= out_degree; i++) {
    c[out_degree - i] = h.coefficients[numeric_cast<size_t>(m) - i] / g.coefficients.back();
    for (size_t j = 0; j <= numeric_cast<size_t>(n); j++)
      h.coefficients[out_degree - i + j] -= c[out_degree - i] * g.coefficients[j];
  }
  h.reduce();
  remainder = h;
  quotient = poly<T>(c);
  quotient.reduce();
  return;
}

template <class T>
poly<T> operator%(const poly<T> &p, const poly<T> &q) {
  poly<T> quot, rem;
  divide(quot, rem, p, q);
  return rem;
}

template <class T>
ostream &operator<<(ostream &outputStream, const poly<T> &p) {
  typename vector<T>::const_iterator i;
  outputStream << "(";
  for (i = p.coefficients.begin(); i != p.coefficients.end() - 1; ++i)
    outputStream << *i << " ";
  outputStream << *(p.coefficients.end() - 1) << ")";
  return outputStream;
}

template <class T>
NumberField<T>::NumberField() {}

template <typename T>
NumberField<T>::NumberField(T p[], size_t deg) : degree(deg) {
  vector<T> coefficients;
  for (size_t i = 0; i <= degree; i++) coefficients.push_back(p[i]);
  minimal_poly = poly<T>(coefficients);
  F_zero = algebraic<T>(this);
  F_one = algebraic<T>(0ul, this);
  build_multiplication_table();
  cross_product_table.resize(degree, degree);
}

template <class T>
NumberField<T>::NumberField(T p[], size_t deg, complex<COORD> emb)
    : embedding(emb), degree(deg) {
  vector<T> coefficients;
  for (size_t i = 0; i <= degree; i++) coefficients.push_back(p[i]);
  minimal_poly = poly<T>(coefficients);
  F_zero = algebraic<T>(this);
  F_one = algebraic<T>(0ul, this);
  build_multiplication_table();
  cross_product_table.resize(degree, degree);
}

template <class T>
void NumberField<T>::build_multiplication_table() {
  vector<T> xtothen, dummy(degree);
  T v;

  for (size_t j = 0; j < degree; j++)
    xtothen.push_back(-minimal_poly.coefficients[j] /
                      minimal_poly.leading_coefficient());

  dummy[0] = T(1) / T(1);

  // dummy[0] = T(1,1);

  for (size_t i = 0; i < degree; i++) {
    multiplication_table.push_back(dummy);
    dummy.insert(dummy.begin(), T(0));
    dummy.erase(dummy.end() - 1);
  }

  for (size_t i = degree; i <= 2 * degree - 2; i++) {
    dummy = multiplication_table[i - 1];
    dummy.insert(dummy.begin(), T(0));
    v = *(dummy.end() - 1);
    dummy.erase(dummy.end() - 1);
    multiplication_table.push_back(dummy + (v * xtothen));
  }
}

template <class T>
poly<T> NumberField<T>::min_poly() {
  return minimal_poly;
}

template <class T>
void NumberField<T>::store_conjugate(algebraic<T> a) {
  conjugate = a;
  conjugate_table.push_back(this->F_one);
  for (size_t i = 1; i < degree; i++) {
    conjugate_table.push_back(a.pow(i));
  }
  build_cross_product_table();
}

template <class T>
algebraic<T>::algebraic() {}

template <class T>
algebraic<T>::algebraic(NumberField<T> *field) : in_field(field) {
  vector<T> v(field->degree);
  coords = v;
}

template <class T>
algebraic<T>::algebraic(size_t n, NumberField<T> *field) {
  vector<T> v(field->degree);
  v[n] = 1;
  coords = v;
  in_field = field;
}

template <class T>
algebraic<T>::algebraic(vector<T> newcoords, NumberField<T> *field)
    : coords(newcoords), in_field(field) {}

template <class T>
algebraic<T>::algebraic(T newcoords[], NumberField<T> *field)
    : in_field(field) {
  for (size_t i = 0; i < field->degree; i++) coords.push_back(newcoords[i]);
}

template <class T>
algebraic<T> algebraic<T>::reciprocal() const {
  poly<T> q(coords), u, v, w;
  w = in_field->minimal_poly;
  gcd_extended(u, v, q, in_field->minimal_poly);
  //    gcd_extended(u, v, q, w);
  return u(algebraic<T>(1, in_field));
}

template <class T>
NumberField<T> *algebraic<T>::field() const {
  return in_field;
}

template <class T>
algebraic<T> operator+(algebraic<T> a, const algebraic<T> &b) {
  //  algebraic<T> c=a;
  return a += b;
}

template <class T>
algebraic<T> operator-(algebraic<T> a, const algebraic<T> &b) {
  //  algebraic<T> c=a;
  return a -= b;
}

template <class T>
algebraic<T> operator/(algebraic<T> a, const algebraic<T> &b) {
  return a /= b;
}

template <class T>
algebraic<T> &algebraic<T>::operator*=(const algebraic<T> &s) {
  vector<T> newcoords(in_field->degree);

  for (size_t i = 0; i < in_field->degree; i++) {
    for (size_t j = 0; j < in_field->degree; j++) {
      T tmp = coords[i] * s.coords[j];
      newcoords += tmp * in_field->multiplication_table[i + j];
      //	  newcoords +=
      //(coords[i]*s.coords[j])*in_field->multiplication_table[i+j];
    }
  }
  coords = newcoords;
  return *this;
}

template <class T>
algebraic<T> &algebraic<T>::operator*=(T q) {
  typename vector<T>::iterator i;
  for (i = coords.begin(); i != coords.end(); ++i) {
    (*i) *= q;
  }
  return (*this);
}

template <class T>
algebraic<T> &algebraic<T>::operator/=(T q) {
  typename vector<T>::iterator i;
  for (i = coords.begin(); i != coords.end(); ++i) {
    (*i) /= q;
  }
  return (*this);
}

template <class T>
algebraic<T> operator*(algebraic<T> a, const algebraic<T> &b) {
  //  algebraic<T> c=a;
  return (a *= b);
}

template <class T>
algebraic<T> operator*(T a, algebraic<T> b) {
  return (b *= a);
}

template <class T>
algebraic<T> operator*(algebraic<T> b, T a) {
  return (b *= a);
}

template <class T>
algebraic<T> operator/(algebraic<T> a, T b) {
  return (a /= b);
}

template <class T>
algebraic<T> &algebraic<T>::operator+=(const algebraic<T> &a) {
  for (unsigned int i = 0; i < coords.size(); i++) coords[i] += a.coords[i];
  return *this;
}

template <class T>
algebraic<T> &algebraic<T>::operator-=(const algebraic<T> &a) {
  for (unsigned int i = 0; i < coords.size(); i++) coords[i] -= a.coords[i];
  return *this;
}

template <class T>
algebraic<T> &algebraic<T>::operator/=(const algebraic<T> &a) {
  return *this *= a.reciprocal();
}

template <class T>
bool operator==(const algebraic<T> &p, const algebraic<T> &q) {
  if (p.in_field != q.in_field) {
    return false;
  }
  if (p.coords == q.coords)
    return true;
  else
    return false;
}

template <class T>
bool operator!=(const algebraic<T> &p, const algebraic<T> &q) {
  return (!(p == q));
}

// return here
template <class T>
algebraic<T> operator-(const algebraic<T> &r) {
  typename vector<T>::const_iterator i;
  vector<T> s;
  for (i = r.coords.begin(); i != r.coords.end(); ++i) s.push_back(-(*i));
  return algebraic<T>(s, r.in_field);
}

// algebraic<T> algebraic<T>::operator -()
// {
//     algebraic<T> r = *this;
//     vector<T>::iterator i;

//     for(i=r.coords.begin(); i!=r.coords.end(); i++) {
// 	*i = -(*i);
//     }
//     return(r);
// }

// algebraic<T> operator ^(algebraic<T> r, int n)
// {
//   algebraic<T> s(0, r.in_field);
//   int i;
//   for(i=1; i<=n; i++)
//     s = s * r;
//   return s;
// }

template <class T>
ostream &operator<<(ostream &outputStream, const algebraic<T> &num) {
  typename vector<T>::iterator i;
  outputStream << "(";
  algebraic<T> tmp = num;
  for (i = tmp.coords.begin(); i != tmp.coords.end() - 1; ++i)
    outputStream << *i << " ";
  outputStream << *(tmp.coords.end() - 1) << ")";
  return outputStream;
}

extern COORD my_mpq_get_d(bigrat op);
extern COORD my_mpq_get_d(int op);
extern COORD my_mpq_get_d(int64_t op);

template <class T>
complex<COORD> algebraic<T>::tocomplex() const {
  complex<COORD> zz(0, 0);
  for (size_t i = 0; i <= in_field->degree - 1; i++)
    zz += my_mpq_get_d(coords[i]) * std::pow(in_field->embedding, i);
  return zz;
}

template <class T>
vector<T> &operator+=(vector<T> &v, vector<T> w) {
  for (unsigned int i = 0; i < v.size(); i++) v[i] += w[i];
  return v;
}

template <class T>
vector<T> operator+(vector<T> v, vector<T> w) {
  vector<T> u;
  for (unsigned int i = 0; i < v.size(); i++) u.push_back(v[i] + w[i]);
  return u;
}

template <class T>
vector<T> operator*(T v, vector<T> w) {
  vector<T> u;
  for (typename vector<T>::iterator i = w.begin(); i != w.end(); ++i)
    u.push_back(v * *i);
  return u;
}

template <class T>
void NumberField<T>::print_mt() {
  typename vector<vector<T> >::iterator i;
  typename vector<T>::iterator j;
  for (i = multiplication_table.begin(); i != multiplication_table.end(); ++i) {
    for (j = (*i).begin(); j != (*i).end(); ++j) cout << *j << " ";
    cout << "\n";
  }
}

template <class T>
void NumberField<T>::print(ostream &out) {
  out << "#Field Degree=" << NumberField<T>::F->degree;
  out << " minimal polynomial=" << NumberField<T>::F->min_poly() << endl;
  out << "#Field generator embedding: "
      << "("
      << boost::lexical_cast<std::string>(NumberField<T>::F->embedding.real());
  out << boost::lexical_cast<std::string>(NumberField<T>::F->embedding.imag());
  out << endl;
}

template <class T>
algebraic<T> algebraic<T>::pow(int n) const {
  algebraic<T> s(0ul, in_field);

  if (n < 0) {
    return (this->reciprocal().pow(-n));
  }

  int i;
  for (i = 1; i <= n; i++) {
    s = s * (*this);
  }
  return s;
}

template <class T>
algebraic<T> algebraic<T>::conjugate() const {
  algebraic<T> conj(in_field);
  for (size_t i = 0; i < in_field->degree; i++)
    conj += (coords[i] * (in_field->conjugate_table)[i]);
  return conj;
}

template <class T>
algebraic<T> algebraic<T>::norm() const  // not the real norm, complex conj only
{
  return ((*this) * (this->conjugate()));
}

template <class T>
algebraic<T> cross_product(const algebraic<T> &u, const algebraic<T> &v) {
  algebraic<T> q(u.in_field);  // set to 0
  T tmp;

  for (size_t i = 0; i < u.in_field->degree; i++) {
    for (size_t j = i + 1; j < u.in_field->degree; j++) {
      tmp = u.coords[i] * v.coords[j] - u.coords[j] * v.coords[i];
      q += u.in_field->cross_product_table(i, j) * tmp;
    }
  }
  //    q = u*v.conjugate() - v*u.conjugate();

  return q;
}

template <typename T>
void NumberField<T>::build_cross_product_table() {
  algebraic<T> xi = algebraic<T>(1, this);

  //    std::vector< boost::array<algebraic<T>, d> > vecs;
  //    vecs.reserve(N);

  // cross_product_table.reserve(d);
  // for(int i=0; i < d; i++ ){
  // 	cross_product_table[i].reserve(vector<algebraic<T> >(d));
  // }

  //    vector< vector< algebraic<T > > >table(d, vector<algebraic<T> >(d));

  for (size_t i = 0; i < degree; i++) {
    //	vector<algebraic<T> > row;
    //	row.clear();
    for (size_t j = 0; j < degree; j++) {  // check sign here
      cross_product_table(i, j) =
          xi.pow(i) * xi.pow(j).conjugate() - xi.pow(j) * xi.pow(i).conjugate();
      //     row.push_back(xi.pow(i)*xi.pow(j).conjugate()
      // 		  - xi.pow(j)*xi.pow(i).conjugate());
    }
    // cross_product_table.push_back(row);
  }
}

algebraic<bigrat> to_rational(
    const algebraic<int64_t> &p)  // only works with given F
{
  assert(p.in_field == NumberField<int64_t>::F);

  vector<bigrat> new_coords;
  for (size_t i = 0; i < p.in_field->degree; i++) {
    static_assert(
        std::is_convertible<int64_t, long>::value,
        "mpz does not implement conversion from long long, so we can only "
        "initialize like this if long and int64_t have the same size; note "
        "that they might not the same type as long == long long and only one "
        "of the two can be aliased to int64_t");
    bigrat r = bigrat(static_cast<long>(p.coords[i]), 1);
    new_coords.push_back(r);
  }

  algebraic<bigrat> q = algebraic<bigrat>(new_coords, NumberField<bigrat>::F);

  return q;
}

algebraic<bigrat> operator/(const algebraic<bigrat> &a, int n) {
  return a * bigrat(1, n);
}

algebraic<bigrat> operator*(const algebraic<bigrat> &r, int n) {
  return r * bigrat(n, 1);
}

algebraic<bigrat> operator*(int n, const algebraic<bigrat> &r) {
  return r * bigrat(n, 1);
}
}  // namespace polygon

// Explicit template instantiations for mpq
namespace polygon {
template class NumberField<bigrat>;
template class algebraic<bigrat>;
template class poly<bigrat>;

template class NumberField<int64_t>;
template class algebraic<int64_t>;
template class poly<int64_t>;

template void gcd_extended(poly<bigrat> &x, poly<bigrat> &y,
                           const poly<bigrat> &a, const poly<bigrat> &b);
template poly<bigrat> operator*(poly<bigrat> p, const poly<bigrat> &q);
template poly<bigrat> operator+(poly<bigrat> p, const poly<bigrat> &q);
template poly<bigrat> operator-(poly<bigrat> p, const poly<bigrat> &q);
template poly<bigrat> operator*(bigrat r, poly<bigrat> p);
template bool operator==(const poly<bigrat> &p, const poly<bigrat> &q);
template bool operator!=(const poly<bigrat> &p, const poly<bigrat> &q);
template void divide(poly<bigrat> &quotient, poly<bigrat> &remainder,
                     const poly<bigrat> &f, const poly<bigrat> &g);

template poly<bigrat> operator%(const poly<bigrat> &p, const poly<bigrat> &q);
template ostream &operator<<(ostream &outputStream, const poly<bigrat> &p);
template algebraic<bigrat> operator+(algebraic<bigrat> a,
                                     const algebraic<bigrat> &b);
template algebraic<bigrat> operator-(algebraic<bigrat> a,
                                     const algebraic<bigrat> &b);
template algebraic<bigrat> operator/(algebraic<bigrat> a,
                                     const algebraic<bigrat> &b);
template algebraic<bigrat> operator*(algebraic<bigrat> a,
                                     const algebraic<bigrat> &b);
template algebraic<bigrat> operator*(bigrat q, algebraic<bigrat> r);
template algebraic<bigrat> operator*(algebraic<bigrat> r, bigrat q);
template algebraic<bigrat> operator/(algebraic<bigrat> a, bigrat b);
template bool operator==(const algebraic<bigrat> &p,
                         const algebraic<bigrat> &q);
template bool operator!=(const algebraic<bigrat> &p,
                         const algebraic<bigrat> &q);
template algebraic<bigrat> operator-(const algebraic<bigrat> &r);
template ostream &operator<<(ostream &outputStream,
                             const algebraic<bigrat> &num);
template vector<bigrat> operator+(vector<bigrat> v, vector<bigrat> w);
template vector<bigrat> operator*(bigrat v, vector<bigrat> w);
template algebraic<bigrat> cross_product(const algebraic<bigrat> &u,
                                         const algebraic<bigrat> &v);
}  // namespace polygon

// Explicit template instantiations for int64
namespace polygon {
template void gcd_extended(poly<int64_t> &x, poly<int64_t> &y,
                           const poly<int64_t> &a, const poly<int64_t> &b);
template poly<int64_t> operator*(poly<int64_t> p, const poly<int64_t> &q);
template poly<int64_t> operator+(poly<int64_t> p, const poly<int64_t> &q);
template poly<int64_t> operator-(poly<int64_t> p, const poly<int64_t> &q);
template poly<int64_t> operator*(int64_t r, poly<int64_t> p);
template bool operator==(const poly<int64_t> &p, const poly<int64_t> &q);
template bool operator!=(const poly<int64_t> &p, const poly<int64_t> &q);
template void divide(poly<int64_t> &quotient, poly<int64_t> &remainder,
                     const poly<int64_t> &f, const poly<int64_t> &g);

template poly<int64_t> operator%(const poly<int64_t> &p,
                                 const poly<int64_t> &q);
template ostream &operator<<(ostream &outputStream, const poly<int64_t> &p);
template algebraic<int64_t> operator+(algebraic<int64_t> a,
                                      const algebraic<int64_t> &b);
template algebraic<int64_t> operator-(algebraic<int64_t> a,
                                      const algebraic<int64_t> &b);
template algebraic<int64_t> operator/(algebraic<int64_t> a,
                                      const algebraic<int64_t> &b);
template algebraic<int64_t> operator*(algebraic<int64_t> a,
                                      const algebraic<int64_t> &b);
template algebraic<int64_t> operator*(int64_t q, algebraic<int64_t> r);
template algebraic<int64_t> operator*(algebraic<int64_t> r, int64_t q);
template bool operator==(const algebraic<int64_t> &p,
                         const algebraic<int64_t> &q);
template bool operator!=(const algebraic<int64_t> &p,
                         const algebraic<int64_t> &q);
template algebraic<int64_t> operator-(const algebraic<int64_t> &r);
template ostream &operator<<(ostream &outputStream,
                             const algebraic<int64_t> &num);
template vector<int64_t> operator+(vector<int64_t> v, vector<int64_t> w);
template vector<int64_t> operator*(int64_t v, vector<int64_t> w);
template algebraic<int64_t> cross_product(const algebraic<int64_t> &u,
                                          const algebraic<int64_t> &v);

}  // namespace polygon
