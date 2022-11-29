#include "array_assign.hpp"

static_assert( ltl::assign_toable<int[2]> );
static_assert( ! ltl::assign_toable<int> );

struct explct { explicit explct() = default; };
static_assert( ltl::empty_list_initializable<int> );
static_assert( ! ltl::empty_list_initializable<explct> );

// Default-assignable tests

// ltl empty_list_assignable concept is implied by this conjunction:
template <typename T>
concept default_init_assignable =
    std::default_initializable<std::remove_reference_t<T>>
 && std::assignable_from<T, T>;

static_assert(   default_init_assignable<int&> );
static_assert( ! default_init_assignable<int> );
static_assert( ! default_init_assignable<int&&> );

static_assert( ! default_init_assignable<int const> );
static_assert( ! default_init_assignable<int const&> );
static_assert( ! default_init_assignable<int const&&> );

static_assert(   ltl::empty_list_assignable<int&> );
static_assert( ! ltl::empty_list_assignable<int> );
static_assert( ! ltl::empty_list_assignable<int&&> );

static_assert( ! ltl::empty_list_assignable<int const> );
static_assert( ! ltl::empty_list_assignable<int const&> );
static_assert( ! ltl::empty_list_assignable<int const&&> );

// An explicit default constructor breaks implicit ={}
struct X { explicit consteval X() = default; };

static_assert( ! default_init_assignable<X> );
static_assert( ! ltl::empty_list_assignable<X> );

struct empty{};

// An operator= overload accepting {}
template <typename...>
struct wrap {
    wrap const& operator=(empty)const{return *this;}
};
template <typename T>
struct wrap<T> {
    T v;
    wrap const& operator=(wrap<>)const{return *this;}
    wrap const& operator=(wrap<std::remove_reference_t<T> const&> w) const
     requires std::is_reference_v<T> {v=w.v;return *this;}
};

auto wemp = wrap<>{} = {};
static_assert( ! std::assignable_from<wrap<>,wrap<>> );

static_assert(   default_init_assignable<wrap<>&> );
static_assert( ! default_init_assignable<wrap<>> );
static_assert( ! default_init_assignable<wrap<>&&> );

static_assert( ltl::empty_list_assignable<wrap<>&> );
static_assert( ltl::empty_list_assignable<wrap<>> );
static_assert( ltl::empty_list_assignable<wrap<>&&> );

static_assert(   default_init_assignable<wrap<int>&> );
static_assert( ! default_init_assignable<wrap<int>> );
static_assert( ! default_init_assignable<wrap<int>&&> );

static_assert( ltl::empty_list_assignable<wrap<int>&> );
static_assert( ltl::empty_list_assignable<wrap<int>> );
static_assert( ltl::empty_list_assignable<wrap<int>&&> );

static_assert( ! default_init_assignable<wrap<int&>&> );
static_assert( ! default_init_assignable<wrap<int&>> );
static_assert( ! default_init_assignable<wrap<int&>&&> );

static_assert( ltl::empty_list_assignable<wrap<int&>&> );
static_assert( ltl::empty_list_assignable<wrap<int&>> );
static_assert( ltl::empty_list_assignable<wrap<int&>&&> );


#define ASSIGNABLE_TO_VAL(L,R,OP) \
   assignable_from<L,R> \
OP assignable_from<L,R&> \
OP assignable_from<L,R&&> \
OP assignable_from<L,R const> \
OP assignable_from<L,R const&> \
OP assignable_from<L,R const&&>

#define ASSIGNABLE_TO_RVAL(L,R,OP) \
   assignable_from<L&&,R> \
OP assignable_from<L&&,R&> \
OP assignable_from<L&&,R&&> \
OP assignable_from<L&&,R const> \
OP assignable_from<L&&,R const&> \
OP assignable_from<L&&,R const&&>

#define ASSIGNABLE_TO_LVAL(L,R,OP) \
   assignable_from<L &,R> \
OP assignable_from<L &,R&> \
OP assignable_from<L &,R&&> \
OP assignable_from<L &,R const> \
OP assignable_from<L &,R const&> \
OP assignable_from<L &,R const&&>

using int2 = int[2];

static_assert( std::assignable_from<int&,long> );

namespace std {
  static_assert(   ASSIGNABLE_TO_LVAL(int,int,&&) );
  static_assert( ! ASSIGNABLE_TO_VAL (int,int,&& !) );
  static_assert( ! ASSIGNABLE_TO_RVAL(int,int,&& !) );
  static_assert( ! ASSIGNABLE_TO_LVAL(int const,int,&& !) );
  static_assert( ! ASSIGNABLE_TO_VAL (int const,int,&& !) );
  static_assert( ! ASSIGNABLE_TO_RVAL(int const,int,&& !) );

  static_assert(   ASSIGNABLE_TO_LVAL(int,long,&&) );

  static_assert( ! ASSIGNABLE_TO_LVAL(int2,int2,&& !) );
}
namespace ltl {
  static_assert(   ASSIGNABLE_TO_LVAL(int,int,&&) );
  static_assert( ! ASSIGNABLE_TO_VAL(int,int,&&!) );
  static_assert( ! ASSIGNABLE_TO_RVAL(int,int,&&!) );
  static_assert( ! ASSIGNABLE_TO_LVAL(int const,int,&& !) );
  static_assert( ! ASSIGNABLE_TO_VAL (int const,int,&& !) );
  static_assert( ! ASSIGNABLE_TO_RVAL(int const,int,&& !) );

  static_assert(   ASSIGNABLE_TO_LVAL(int2,int2,&&) );
}
