// First, test the header "ALLOW_ZERO_SIZE_ARRAY.hpp"

// This test should be compiled with warnings-as-errors (-Werror or /WX) 
// to check that warnings are silenced on GCC, Clang and MSVC
// failures are then caught as compile failures and are noticed.
// But, this is likely to be flaky with different compiler releases.

// It isn't recommended to compile dependent code with warnings-as-errors;
// too hard for code that may use zero-size arrays.

#include <cassert>
using size_t = decltype(sizeof"");

// struct A { T a[0]; };
// Zero-size array member.
// A final zero-size array struct-member is allowed, as an old C extension.
// MSVC allows a zero-size array as the final member only.
// GCC and Clang allow multiple zero-size arrays in any position which means
// that their addresses can alias. [[no_unique_address]] has no effect.
// A zero-size array can be the sole member (on compilers tested so far).
// (C99 FAM specifies that a final T[] member should have prior member[s]).
//
struct Array
{
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
int data[0];
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
};

// GCC and Clang don't require guards for using user-defined types containing
// zero-size arrays. MSVC requires a declaration of Array UDT to be guarded.
//
#ifdef _MSC_VER
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
#endif
Array A0;
#ifdef _MSC_VER
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
#endif

// sizeof Array
// The Array UDT is sizeof == 0 on GCC and Clang
// on MSVC it appears that sizeof is the sizeof the element type
//
#ifndef _MSC_VER
static_assert( sizeof(Array) == 0 );
static_assert( sizeof A0 == 0 );
#else
static_assert( sizeof(Array) == sizeof(int) );
static_assert( sizeof A0 == sizeof(int) );
#endif

// same for template UDT
template <int N> struct array
{
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
int data[N];
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
};

array<1> a1; // fine
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
array<0> a0;
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"

// using A = T[0];
// Zero-size array type-alias declarations with explicit array syntax
// require guards to suppress warnings on GCC, Clang and MSVC
//
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
using int0 = int[0];
using size_t0 = size_t[0];
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"

// sizeof(T[0])
// A hard error on MSVC.
// Evaluates to 0 on GCC and Clang, where
// sizeof(T[0]) explicit-array-syntax requires a guard
// sizeof type-alias or variable requires no guard
//
#ifndef _MSC_VER
static_assert(sizeof(int0) == 0);
static_assert(sizeof int0{} == 0);
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
static_assert(sizeof(int[0]) == 0);
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
#endif

// Uninitialized global zero-size arrays can be declared with a type alias
// without guards on GCC, Clang and MSVC
//
extern int0 e;
int0 s; // is this a definition, or just a declaration -> unresolved external

// T x[0];
// GCC and Clang may warn on global or local declarations
// Zero-size array declaration is a hard error on MSVC -
// error C2466: cannot allocate an array of constant size 0
//
#ifndef _MSC_VER
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
extern int e0[0];
int s0[0];
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
#endif

// Similar to above, an intitialized zero-size array is a hard-fail on MSVC
//
#ifndef _MSC_VER
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
int0 j0{};
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
#endif

// MSVC appears to totally forbid local zero-size arrays,
// (even when declared with an alias and unitialized, which works for globals)
// GCC and Clang accept zero-size locals; addresses may alias.
//
auto locals = [] {
#ifndef _MSC_VER
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
  int loci0[0];
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
  int0 locj0;
//  assert( &loci0 == &locj0);
#endif
};

// A workaround for not being able to declare a zero-size array on MSVC
// is to declare a reference-to-array with lifetime extension of the temporary.
// Works for globals, with no guards needed for now...
//
int0&& zr = {};
//
// Zero-size array refs can be passed to functions and returned, with no guards
// (this cryptic expression is the id identity operation)
//
auto&& zc = [](auto&&a)->auto&&{return (decltype(a))a;}(zr);

// The address comparison is not constexpr on MSVC (asserted in main)
// And MSVC doesn't allow constexpr reference-to-array temporary
//
#ifndef _MSC_VER
static_assert(sizeof zr == 0 && sizeof zc == 0 );
static_assert( &zr == &zc );
constexpr int0&& yr = {};
#endif

// A zero-size array can be captured
// (MSVC needs guards enclosing the whole function here)
//
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
auto closure(auto&& a)
{
  auto x = [a]{return a;};
  return x;
}
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"

// As with the Array class above, sizeof closure == 0 on GCC and Clang
// On MSVC it appears that sizeof is the sizeof the element type
//
#ifndef _MSC_VER
static_assert( sizeof closure(int0{}) == 0 );
static_assert( sizeof closure(size_t0{}) == 0 );
#else
static_assert( sizeof closure(int0{}) == sizeof(int) );
static_assert( sizeof closure(size_t0{}) == sizeof(size_t) );
#endif


int main()
{
  locals();
  assert( &zr == &zc );
}
