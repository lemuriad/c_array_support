#include "c_array_support.hpp"

using X = class X { X() = delete; }; // non-default constructible

using int2 = int[2];
using int23 = int[2][3];

auto&& array_id(ltl::c_array auto&& a) { return (decltype(a))a; }

#include "ALLOW_ZERO_SIZE_ARRAY.hpp"

using X0 = X[0];
using X01 = X[0][1];
using X012 = X[0][1][2];
using int0 = int[0];
using int01 = int[0][1];
using int012 = int[0][1][2];

#include "ALLOW_ZERO_SIZE_ARRAY.hpp"

static_assert( ltl::rank_v<X> == 0 );
static_assert( ltl::rank_v<X0> == 1 );
static_assert( ltl::rank_v<X01> == 2 );
static_assert( ltl::rank_v<X012> == 3 );


static_assert( ltl::c_array<int0> );
static_assert( ltl::c_array<int0&> );
static_assert( ltl::c_array<X0> );
static_assert( ltl::c_array<X0 const&&> );

static_assert( ltl::c_array<int0,int> );
static_assert( ltl::c_array<int0&,int> );
static_assert( ltl::c_array<X0,X> );
static_assert( ltl::c_array<X0 const&&,X> );

static_assert( ! ltl::c_array<int0,int const> );
static_assert( ! ltl::c_array<int0&,int&> );
static_assert( ! ltl::c_array<X0,X volatile> );
static_assert( ! ltl::c_array<X0 const&&,X const> );

ltl::c_array auto& hi = "hi";
ltl::c_array<char> auto& ho = hi;

static_assert( ltl::c_array<int[2]> );
static_assert( ltl::c_array<int(&)[2]> );
static_assert( ltl::c_array<int const(&&)[2]> );

// c_array does not model unbounded array
static_assert( ! ltl::c_array<int(&)[]> );
static_assert( ! ltl::c_array<int const(&)[]> );
static_assert( ! ltl::c_array<int const volatile[]> );

static_assert( ! ltl::c_array<void> );
static_assert( ! ltl::c_array<int> );
static_assert( ! ltl::c_array<int&> );

static_assert(  ltl::is_bounded_array_v<int[1]> && ltl::is_array_v<int[1]> );
static_assert(  ltl::is_bounded_array_v<X[1]>   && ltl::is_array_v<X[1]> );

static_assert(  ltl::is_bounded_array_v<int0>   && ltl::is_array_v<int0> );
static_assert(  ltl::is_bounded_array_v<X0>     && ltl::is_array_v<X0> );

static_assert( ! ltl::is_bounded_array_v<int[]> && ltl::is_array_v<int[]> );

static_assert( ! ltl::is_bounded_array_v<void>     && ! ltl::is_array_v<void> );

static_assert( ! ltl::is_bounded_array_v<int>      && ! ltl::is_array_v<int> );
static_assert( ! ltl::is_bounded_array_v<X>        && ! ltl::is_array_v<X> );
static_assert( ! ltl::is_bounded_array_v<int*>     && ! ltl::is_array_v<int*> );
static_assert( ! ltl::is_bounded_array_v<int&>     && ! ltl::is_array_v<int&> );
static_assert( ! ltl::is_bounded_array_v<int(&)()> && ! ltl::is_array_v<int(&)()> );
static_assert( ! ltl::is_bounded_array_v<int(&)[]> && ! ltl::is_array_v<int(&)[]> );
static_assert( ! ltl::is_bounded_array_v<int()>    && ! ltl::is_array_v<int()> );

// flat_size<A> trait tests
static_assert( ltl::flat_size<int0> == 0 );

static_assert( ltl::flat_size<int const(&&)[2]> == 2 );
static_assert( ltl::flat_size<int[2][3][4][5]> == 120 );
static_assert( ltl::flat_size<char[1][2][3][4][5][6]> == 720 );
static_assert( ltl::flat_size<int[1][2][3][4][5][6][7]> == 5040 );

static_assert( std::is_same_v<ltl::flat_cast_t<int[2][3]>,
                                               int[6]> );
static_assert( std::is_same_v<ltl::flat_cast_t<int const[2][3]>,
                                               int const[6]> );
static_assert( std::is_same_v<ltl::flat_cast_t<int const(&&)[2][3]>,
                                               int const(&&)[6]> );

// same_extents<A,B> trait tests

static_assert(  ltl::same_extents<char[3][4], int[3][4]> );
static_assert( ! ltl::same_extents<int[3][4], int[3][3]> );
static_assert( ! ltl::same_extents<int[3][4], int[4][4]> );

// c_array_t<T,N...> trait test

static_assert( std::is_same_v< ltl::c_array_t<int const,1,2,3>,
                                             int const[1][2][3]> );

static_assert( std::is_same_v<ltl::remove_extent_t<X0>,X> );
static_assert( std::is_same_v<ltl::remove_extent_t<X01>,X[1]> );
static_assert( std::is_same_v<ltl::remove_extent_t<X012>,X[1][2]> );

static_assert( std::is_same_v<ltl::remove_extent_t<void>,void> );
static_assert( std::is_same_v<ltl::remove_extent_t<void()>,void()> );
static_assert( std::is_same_v<ltl::remove_extent_t<int>,int> );
static_assert( std::is_same_v<ltl::remove_extent_t<int*>,int*> );
static_assert( std::is_same_v<ltl::remove_extent_t<int&>,int&> );
static_assert( std::is_same_v<ltl::remove_extent_t<int&&>,int&&> );

static_assert( std::is_same_v<ltl::remove_all_extents_t<X0>,X> );
static_assert( std::is_same_v<ltl::remove_all_extents_t<X01>,X> );
static_assert( std::is_same_v<ltl::remove_all_extents_t<X012>,X> );

#ifndef _MSC_VER
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
 // MSVC doesn't support zero bounds beyond first
using X00 = X[0][0];
using X10 = X[1][0];
using X20 = X[2][0];
using X02 = X[0][2];
using X001 = X[0][0][1];
using X100 = X[1][0][0];
using X102 = X[1][0][2];
using X120 = X[1][2][0];

static_assert( std::is_same_v<ltl::c_array_t<X,0,0>,X00> );

static_assert( ltl::c_array<X00> );
static_assert( ltl::c_array<X10> );
static_assert( ltl::c_array<X001> );
static_assert( ltl::c_array<X100> );
static_assert( ltl::c_array<X102> );
static_assert( ltl::c_array<X120> );

static_assert( ltl::c_array_unpadded<X10> );
static_assert( ltl::c_array_unpadded<X001> );
static_assert( ltl::c_array_unpadded<X100> );
static_assert( ltl::c_array_unpadded<X102> );
static_assert( ltl::c_array_unpadded<X120> );

static_assert( ltl::flat_size<X00> == 0 );
static_assert( ltl::flat_size<X10> == 0 );
static_assert( ltl::flat_size<X001> == 0 );
static_assert( ltl::flat_size<X100> == 0 );
static_assert( ltl::flat_size<X102> == 0 );
static_assert( ltl::flat_size<X120> == 0 );

static_assert( ltl::rank_v<X00> == 2 );
static_assert( ltl::rank_v<X10> == 2 );
static_assert( ltl::rank_v<X001> == 3 );
static_assert( ltl::rank_v<X100> == 3 );
static_assert( ltl::rank_v<X102> == 3 );
static_assert( ltl::rank_v<X120> == 3 );

static_assert(   ltl::same_extents<X00,X00> );
static_assert(   ltl::same_extents<X10,X10> );
static_assert(   ltl::same_extents<X102,X102> );
static_assert(   ltl::same_extents<X120,X120> );
static_assert( ! ltl::same_extents<X10,X01> );
static_assert( ! ltl::same_extents<X102,X120> );
static_assert( ! ltl::same_extents<X012,X120> );

static_assert( std::is_same_v<ltl::remove_extent_t<X00>,X0> );
static_assert( std::is_same_v<ltl::remove_extent_t<X10>,X0> );
static_assert( std::is_same_v<ltl::remove_extent_t<X001>,X01> );
static_assert( std::is_same_v<ltl::remove_extent_t<X100>,X00> );
static_assert( std::is_same_v<ltl::remove_extent_t<X102>,X02> );
static_assert( std::is_same_v<ltl::remove_extent_t<X120>,X20> );

static_assert( std::is_same_v<ltl::remove_all_extents_t<X00>, X> );
static_assert( std::is_same_v<ltl::remove_all_extents_t<X10>, X> );
static_assert( std::is_same_v<ltl::remove_all_extents_t<X001>,X> );
static_assert( std::is_same_v<ltl::remove_all_extents_t<X100>,X> );
static_assert( std::is_same_v<ltl::remove_all_extents_t<X102>,X> );
static_assert( std::is_same_v<ltl::remove_all_extents_t<X120>,X> );

#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
#endif

// subscript(a,i) tests

extern int eint2[2];
inline constexpr int cint2[2] {1,2};

static_assert( ltl::subscript(int2{1,2}) == 1 );
static_assert( ltl::subscript(int2{1,2},1) == 2 );
static_assert( ltl::subscript(cint2) == 1 );
static_assert( ltl::subscript(cint2,1) == 2 );

static_assert( std::is_same_v< decltype(ltl::subscript(int2{})),
                               int&&> );
static_assert( std::is_same_v< decltype(ltl::subscript(eint2)),
                               int&> );
static_assert( std::is_same_v< decltype(ltl::subscript(cint2)),
                               int const&> );

// flat_index(a,i) tests

static_assert( &ltl::flat_index(eint2) == &ltl::subscript(eint2) );
static_assert( &ltl::flat_index(eint2) == &eint2[0] );

static_assert( ltl::flat_index(int2{1,2}) == 1 );
static_assert( ltl::flat_index(int2{1,2},1) == 2 );
static_assert( ltl::flat_index(cint2) == 1 );
static_assert( ltl::flat_index(cint2,1) == 2 );

static_assert( std::is_same_v< decltype(ltl::flat_index(eint2)),
                               int&> );
static_assert( std::is_same_v< decltype(ltl::flat_index(cint2)),
                               int const&> );

extern int eint23[2][3];
inline constexpr int cint23[2][3] {{1,2,3},{4,5,6}};

static_assert( ltl::flat_index(int23{{1,2,3},{4,5,6}}) == 1 );
static_assert( ltl::flat_index(int23{{1,2,3},{4,5,6}},3) == 4 );

static_assert( ltl::flat_index(cint23) == 1 );
static_assert( ltl::flat_index(cint23,4) == 5 );


inline constexpr int cint4213[4][2][1][3]
{  { {{0,1,2}},{{3,4,5}} },
   { {{6,7,8}},{{9,0,1}} },
   { {{2,3,4}},{{5,6,7}} },
   { {{8,9,0}},{{1,2,3}} } };

static_assert( ltl::flat_index(cint4213,0) == 0 );
static_assert( ltl::flat_index(cint4213,1) == 1 );
static_assert( ltl::flat_index(cint4213,2) == 2 );
static_assert( ltl::flat_index(cint4213,3) == 3 );
static_assert( ltl::flat_index(cint4213,5) == 5 );
static_assert( ltl::flat_index(cint4213,6) == 6 );
static_assert( ltl::flat_index(cint4213,8) == 8 );
static_assert( ltl::flat_index(cint4213,23) == 3 );

static_assert( std::is_same_v<ltl::all_extents_removed_t<int[2][3]>,
                                                  int> );

static_assert( std::is_same_v<ltl::all_extents_removed_t<int const[2][3]>,
                                                  int const> );

static_assert( std::is_same_v<ltl::all_extents_removed_t<int(&)[2][3]>,
                                                  int&> );

static_assert( std::is_same_v<ltl::all_extents_removed_t<int const(&)[2][3]>,
                                                  int const&> );

static_assert( std::is_same_v<ltl::all_extents_removed_t<int(&&)[2][3]>,
                                                  int&&> );

static_assert( std::is_same_v<ltl::all_extents_removed_t<int const(&&)[2][3]>,
                                                  int const&&> );
