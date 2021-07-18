#include "c_array_support.hpp"

// c_array<T> concept tests

static_assert( ltl::c_array<int[2]> );
static_assert( ltl::c_array<int(&)[2]> );
static_assert( ltl::c_array<int const(&&)[2]> );
static_assert( ltl::c_array<int[]> );
static_assert( ltl::c_array<int const volatile[]> );

static_assert( ! ltl::c_array<int> );
static_assert( ! ltl::c_array<int&> );

// flat_size<A> trait tests

static_assert( ltl::flat_size<int const(&&)[2]> == 2 );
static_assert( ltl::flat_size<int[2][3][4][5]> == 120 );
static_assert( ltl::flat_size<char[1][2][3][4][5][6]> == 720 );
static_assert( ltl::flat_size<int[1][2][3][4][5][6][7]> == 5040 );

static_assert( std::is_same_v<ltl::flat_cast_t<int const(&&)[2][3]>,
                                               int const[6]> );

// same_extents_v<A,B> trait tests

static_assert(  ltl::same_extents_v<char[3][4], int[3][4]> );
static_assert( ! ltl::same_extents_v<int[3][4], int[3][3]> );
static_assert( ! ltl::same_extents_v<int[3][4], int[4][4]> );

// c_array_t<T,N...> trait test

static_assert( std::is_same_v< ltl::c_array_t<int const,1,2,3>,
                                             int const[1][2][3]> );

using int2 = int[2];
extern int eint2[2];
inline constexpr int cint2[2] {1,2};

// subscript(a,i) tests

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

static_assert( ltl::flat_index(int2{1,2}) == 1 );
static_assert( ltl::flat_index(int2{1,2},1) == 2 );
static_assert( ltl::flat_index(cint2) == 1 );
static_assert( ltl::flat_index(cint2,1) == 2 );

static_assert( std::is_same_v< decltype(ltl::flat_index(int2{})),
                               int&&> );
static_assert( std::is_same_v< decltype(ltl::flat_index(cint2)),
                               int const&> );
static_assert( std::is_same_v< decltype(ltl::flat_index(cint2)),
                               int const&> );

using int23 = int[2][3];
extern int eint23[2][3];
inline constexpr int cint23[2][3] {{1,2,3},{4,5,6}};

static_assert( ltl::flat_index(int23{{1,2,3},{4,5,6}}) == 1 );
static_assert( ltl::flat_index(int23{{1,2,3},{4,5,6}},3) == 4 );

static_assert( ltl::flat_index(cint23) == 1 );
static_assert( ltl::flat_index(cint23,4) == 5 );

static_assert( ltl::flat_index(int2{1,2},1) == 2 );
static_assert( ltl::flat_index(cint2,1) == 2 );

static_assert( std::is_same_v< decltype(ltl::flat_index(int2{})),
                               int&&> );
static_assert( std::is_same_v< decltype(ltl::flat_index(cint2)),
                               int const&> );
static_assert( std::is_same_v< decltype(ltl::flat_index(cint2)),
                               int const&> );

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

static_assert( std::is_same_v<ltl::flat_element_t<int[2][3]>,
                                                  int> );

static_assert( std::is_same_v<ltl::flat_element_t<int const[2][3]>,
                                                  int const> );

static_assert( std::is_same_v<ltl::flat_element_t<int(&)[2][3]>,
                                                  int&> );

static_assert( std::is_same_v<ltl::flat_element_t<int const(&)[2][3]>,
                                                  int const&> );

static_assert( std::is_same_v<ltl::flat_element_t<int(&&)[2][3]>,
                                                  int&&> );

static_assert( std::is_same_v<ltl::flat_element_t<int const(&&)[2][3]>,
                                                  int const&&> );

