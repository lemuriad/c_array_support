#include "array_compare.hpp"

#if defined (_MSC_VER)
#define EXCLUDE_MSVC(...)
#else
#define EXCLUDE_MSVC(...) __VA_ARGS__
#endif

using lml::impl::pointer_equality_comparable_with;

static_assert( lml::compare_three_way{}("hello","world") < 0 );
//static_assert( std::compare_three_way{}("hello","world") < 0 );

constexpr int a01_23[2][2] {{0,1},{2,3}};
constexpr int a01_22[2][2] {{0,1},{2,2}};
static_assert( lml::compare_three_way{}(a01_23, a01_22) > 0 );

EXCLUDE_MSVC(
static_assert( lml::compare_three_way{}(a01_23, {{0,1},{2,2}}) > 0 );
)

using I2 = int[2];
using L2 = long[2];

inline constexpr I2 a{0,1}, aa{0,1};
inline constexpr L2 b{0,1};

static_assert( std::equality_comparable_with<int*,int*> );
static_assert( lml::equality_comparable_with<int*,int*> );
static_assert( pointer_equality_comparable_with<int*,int*> );

static_assert( std::equality_comparable<int[2]> ); // id equality
static_assert( lml::equality_comparable<int[2]> ); // array equality

static_assert( std::equality_comparable_with<int[2],int[4]> );
static_assert( lml::equality_comparable_with<int[2],int[2]> );
static_assert( ! lml::equality_comparable_with<int[2],int[4]> );
static_assert( ! pointer_equality_comparable_with<int[2],int[4]> );

static_assert( lml::same_extents<I2,L2> );

static_assert( ! std::three_way_comparable<I2>
              && lml::three_way_comparable<I2>);

static_assert( ! std::three_way_comparable_with<I2,I2>
              && lml::three_way_comparable_with<I2,I2>);

static_assert( ! std::three_way_comparable_with<I2,I2>
              && lml::three_way_comparable_with<I2,L2>
              && lml::three_way_comparable_with<I2 const& ,L2 const&>);

static_assert( lml::equality_comparable<I2>);

static_assert( ! std::equality_comparable_with<I2,L2>
              && lml::equality_comparable_with<I2,L2>);

static_assert( lml::totally_ordered<I2>);

static_assert( ! std::totally_ordered_with<I2,L2>
              && lml::totally_ordered_with<I2,L2>);

static_assert( std::totally_ordered_with<double(&)[2],double*> );
static_assert( lml::totally_ordered_with<double(&)[2],double[2]> );
static_assert(!lml::totally_ordered_with<double(&)[2],double*> );

static_assert( std::equality_comparable<std::nullptr_t> 
            && lml::equality_comparable<std::nullptr_t> );

static_assert( ! std::three_way_comparable<std::nullptr_t> 
            && ! lml::three_way_comparable<std::nullptr_t> );

static_assert( lml::equal_to{}(std::nullptr_t{}, std::nullptr_t{}) );

static_assert( lml::compare_three_way{}(aa    ,a) == 0);

static_assert( lml::compare_three_way{}(I2{0,1},I2{0,1}) == 0);
static_assert( lml::compare_three_way{}(I2{0,1},I2{1,0}) < 0);
static_assert( lml::compare_three_way{}(I2{0,1},I2{0,0}) > 0);

static_assert( lml::compare_three_way{}(I2{0,1},a) == 0);
static_assert( lml::compare_three_way{}(a,     I2{0,1}) == 0);
static_assert( lml::compare_three_way{}(a,     I2{1,0}) < 0);
static_assert( lml::compare_three_way{}(I2{0,1},I2{0,0}) > 0);

static_assert( lml::compare_three_way{}(I2{0,1}, {0,1}) == 0);
static_assert( lml::compare_three_way{}(I2{0,1}, {1,0}) < 0);
static_assert( lml::compare_three_way{}(I2{0,1}, {0,0}) > 0);

static_assert( lml::compare_three_way{}(I2{0,1},L2{0,1}) == 0);
static_assert( lml::compare_three_way{}(I2{0,1},L2{1,0}) < 0);
static_assert( lml::compare_three_way{}(I2{0,1},L2{0,0}) > 0);

static_assert( lml::equal_to{}(a,a) );
static_assert( lml::equal_to{}(a,aa) );

static_assert( lml::equal_to{}(I2{0,1},I2{0,1}) );
static_assert( ! lml::equal_to{}(I2{0,1},I2{1,0}) );
static_assert( ! lml::equal_to{}(I2{0,1},I2{0,0}) );

static_assert( lml::equal_to{}(a,b) );
static_assert( lml::equal_to{}(I2{0,1},L2{0,1}) );

static_assert(   lml::member_default_3way<int> );
static_assert(   lml::member_default_3way<int[2]>
              || lml::impl::GCC10_ARRAY_COMPARE_WORKAROUND);
static_assert(   lml::member_default_3way<int*> );
static_assert( ! lml::member_default_3way<int&> );

static_assert(   lml::member_default_equality<int> );
static_assert(   lml::member_default_equality<int[2]>
              || lml::impl::GCC10_ARRAY_COMPARE_WORKAROUND);
static_assert(   lml::member_default_equality<int*> );
static_assert( ! lml::member_default_equality<int&> );

enum { N = -1, Z = 0, P = +1 };

inline constexpr 
int less_data[][3]{
  {N,N,N},  {N,N,Z},  {N,N,P},
  {N,Z,N},  {N,Z,Z},  {N,Z,P},
  {N,P,N},  {N,P,Z},  {N,P,P},

  {Z,N,N},  {Z,N,Z},  {Z,N,P},
  {Z,Z,N},  {Z,Z,Z},  {Z,Z,P},
  {Z,P,N},  {Z,P,Z},  {Z,P,P},

  {P,N,N},  {P,N,Z},  {P,N,P},
  {P,Z,N},  {P,Z,Z},  {P,Z,P},
  {P,P,N},  {P,P,Z},  {P,P,P} };

#include <algorithm>

static_assert( std::ranges::is_sorted( less_data, lml::less{} ) );
