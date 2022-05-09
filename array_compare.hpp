//  Copyright (c) 2021 Will Wray https://keybase.io/willwray
//
//  Distributed under the Boost Software License, Version 1.0.
//        http://www.boost.org/LICENSE_1_0.txt
//
//  Repo: https://github.com/willwray/c_array_support

#ifndef LTL_ARRAY_COMPARE_HPP
#define LTL_ARRAY_COMPARE_HPP
/*
  array_compare.hpp
  =================

  Replacements for C++20 std library facilities for generic comparison
  extended to support C arrays. Only same-size, same shape, arrays are
  considered comparable. Multidimensional arrays are compared as-if flat.
  
  Depends on <compare> and c_array_support.hpp

  Avoids <algorithm> or <functional> dependency by implementing algorithms
  similar to std::lexicographical_compare_three_way, ranges equality, etc.
  tailored to same-shape C arrays, using flat indexing rather than recursion
  (currently lacking optimizations such as memcmp specializations).

  Concepts:

    ltl::three_way_comparable[_with]  c.f. std::three_way_comparable
    ltl::equality_comparable[_with]   c.f. std::equality_comparable
    ltl::totally_ordered[_with]       c.f. std::totally_ordered

  Aliases:
    ltl::compare_three_way_result_t c.f. std::compare_three_way_result_t

  Functors:
    ltl::compare_three_way     c.f. std::compare_three_way
    ltl::equal_to              c.f. std::ranges::equal_to
    ltl::not_equal_to          c.f. std::ranges::not_equal_to
    ltl::less                  c.f. std::ranges::less

  Usage
  =====
  Usage is the same as the std versions, except that arrays are compared:

      constexpr char hello[] = "hello";
      std::ranges::equal_to{}( "hello", hello );  // false, warning if lucky
      assert( ltl::equal_to{}( "hello", hello) ); // true

      std::ranges::less{}("hello","world"); // indeterminate, warning if lucky
      assert( ltl::less{}("hello","world") ); // true

      std::compare_three_way{}("hello","world"); // compile fail
      ltl::compare_three_way{}("hello","world") < 0; // true

      int a[2][2] {{0,1},{2,3}}, b[2][2] {{0,1},{2,3}};

      ltl::compare_three_way{}( a, b ) > 0;
      ltl::compare_three_way{}( a, {{0,1},{2,2}} ) > 0;

  The ltl functors accept rvalue array ^^^ braced-initializer list as the RHS.
  See the ltl::tupl implementation for example usage in array member support.

  Raison d'etre
  =============
  C array is not comparable, on its own, and C++20 doesn't go far enough
  to fix array comparison or to assist generic code in comparing arrays.

  C++20 introduces operator<=> for three way comparison and allows
  classes to default <=> and/or == including for array members.

  The std::compare_three_way functor defined in <compare> and the ranges
  comparison functors std::ranges::equal_to, etc., do not support array,
  so this header provides lookalikes that support C array.

  (Array-array comparison is deprecated in C++20, so could be removed in
   future and then fixed. Hopefully this header is a stopgap till then.)
*/

#include <compare>

#include "c_array_support.hpp"

#ifndef __UINTPTR_TYPE__
#include <cstdint>
#  define UINTPTR uintptr_t
#else
#  define UINTPTR __UINTPTR_TYPE__
#endif

#include "namespace.hpp"

// Comparison concepts from std lib extended to include array type
//   three_way_comparable[_with]
//   equality_comparable[_with]
//   totally_ordered[_with]
//
template <typename A, class Cat = std::partial_ordering>
concept three_way_comparable =
   std::three_way_comparable<all_extents_removed_t<A>, Cat>;

template <typename L, typename R, class Cat = std::partial_ordering>
concept three_way_comparable_with =
   std::three_way_comparable_with<all_extents_removed_t<L>,
                                  all_extents_removed_t<R>, Cat>
  && same_extents<std::remove_cvref_t<L>,std::remove_cvref_t<R>>;

template <typename A>
concept equality_comparable =
   std::equality_comparable<all_extents_removed_t<A>>;

template <typename L, typename R>
concept equality_comparable_with =
   std::equality_comparable_with<all_extents_removed_t<L>,
                                 all_extents_removed_t<R>>
  && same_extents<std::remove_cvref_t<L>,std::remove_cvref_t<R>>;

template <typename A>
concept totally_ordered =
   std::totally_ordered<all_extents_removed_t<A>>;

template <typename L, typename R>
concept totally_ordered_with =
   std::totally_ordered_with<all_extents_removed_t<L>,
                             all_extents_removed_t<R>>
  && same_extents<std::remove_cvref_t<L>,std::remove_cvref_t<R>>;

template <typename L, typename R = L>
using compare_three_way_result_t = 
 std::compare_three_way_result_t<all_extents_removed_t<L>,
                                 all_extents_removed_t<R>>;

namespace impl {
// pointer_equality_comparable_with
//   A helper concept for ltl::equal_to and ltl::not_equal_to
//   c.f. pointer equality requirements for std::ranges::equal_to
//
template <typename P, typename Q>
concept pointer_equality_comparable_with =
     ! c_array<P> && ! c_array<Q>
  && requires (P&& l, Q&& r) { {l == r} -> std::same_as<bool>; }
  && std::convertible_to<P, const volatile void*>
  && std::convertible_to<Q, const volatile void*>
  && (! requires(P&& l, Q&& r) { operator==(static_cast<P&&>(l),
                                            static_cast<Q&&>(r)); }
      && ! requires(P&& l, Q&& r)
           { static_cast<P&&>(l).operator==(static_cast<Q&&>(r)); }
     );

// pointer_less_than_comparable_with, as above for ltl::less functor
//
template <typename P, typename Q>
concept pointer_less_than_comparable_with =
     ! c_array<P> && ! c_array<Q>
  && requires (P&& l, Q&& r) { {l < r} -> std::same_as<bool>; }
  && std::convertible_to<P, const volatile void*>
  && std::convertible_to<Q, const volatile void*>
  && (! requires(P&& l, Q&& r) { operator<(static_cast<P&&>(l),
                                           static_cast<Q&&>(r)); }
      && ! requires(P&& l, Q&& r)
           { static_cast<P&&>(l).operator<(static_cast<Q&&>(r)); }
     );
} // impl

// compare_three_way
//   A version of std::compare_three_way extended to compare arrays
//
struct compare_three_way
{
  template <typename L, typename R>
    requires three_way_comparable_with<L,R>
  constexpr auto operator()(L&& l, R&& r) const
    noexcept(noexcept(std::compare_three_way{}(flat_index((L&&)l),
                                               flat_index((R&&)r)) ))
    -> compare_three_way_result_t<L,R>
  {
    if constexpr (std::three_way_comparable_with<L,R>)
      return std::compare_three_way{}((L&&)l, (R&&)r);
    else
    {
      for (int i = 0; i != flat_size<L>; ++i)
      {
        auto c = std::compare_three_way{}(flat_index((L&&)l,i),
                                          flat_index((R&&)r,i));
        if (c != 0)
          return c;
      }
      return compare_three_way_result_t<L,R>::equivalent;
    }
  }

  template <typename A>
  constexpr auto operator()(A const& l, A const& r) const noexcept(
    noexcept(operator()<A const&, A const&>(l,r)))
  {
    return operator()<A const&, A const&>(l,r);
  }

  using is_transparent = void;
};

// equal_to functor corrected to compare arrays (rather than array ids)
struct equal_to
{
  template <typename L, typename R>
    requires (equality_comparable_with<L,R>
           || impl::pointer_equality_comparable_with<all_extents_removed_t<L>,
                                                     all_extents_removed_t<R>>)
  constexpr bool operator()(L&& l, R&& r) const
    noexcept(noexcept(flat_index((L&&)l) == flat_index((R&&)r)))
  {
    if constexpr (! c_array<L>)
      return (L&&)l == (R&&)r;
    else
    {
      for (int i = 0; i != flat_size<L>; ++i)
        if ( flat_index((L&&)l,i) != flat_index((R&&)r,i) )
          return false;
      return true;
    }
  }

  template <typename A>
  constexpr bool operator()(A const& l, A const& r) const noexcept(
    noexcept(operator()<A const&, A const&>(l,r)))
  {
    return operator()<A const&, A const&>(l,r);
  }

  using is_transparent = void;
};

// not_equal_to functor corrected to compare arrays (rather than array ids)
struct not_equal_to
{
  template <typename L, typename R>
    requires (equality_comparable_with<L,R>
           || impl::pointer_equality_comparable_with<all_extents_removed_t<L>,
                                                     all_extents_removed_t<R>>)
  constexpr bool operator()(L&& l, R&& r) const
    noexcept(noexcept(flat_index((L&&)l) == flat_index((R&&)r)))
  {
    return ! equal_to{}((L&&)l, (R&&)r);
  }

  template <typename A>
  constexpr bool operator()(A const& l, A const& r) const noexcept(
    noexcept(operator()<A const&, A const&>(l,r)))
  {
    return operator()<A const&, A const&>(l,r);
  }

  using is_transparent = void;
};


// less functor corrected to compare arrays (rather than array ids)
struct less
{
  template <typename L, typename R>
    requires (totally_ordered_with<L,R>
          || impl::pointer_less_than_comparable_with<all_extents_removed_t<L>,
                                                     all_extents_removed_t<R>>)
  constexpr bool operator()(L&& l, R&& r) const
    noexcept(noexcept(flat_index((L&&)l) < flat_index((R&&)r)))
  {
    if constexpr (impl::pointer_less_than_comparable_with<L,R>)
    {
      if (std::is_constant_evaluated())
        return l < r;
      else
        return
          reinterpret_cast<UINTPTR>(static_cast<const volatile void*>((L&&)l))
        < reinterpret_cast<UINTPTR>(static_cast<const volatile void*>((R&&)r));
    }
    else if constexpr (! c_array<L>)
    {
      return (L&&)l < (R&&)r;
    }
    else
    {
      for (int i = 0; i != flat_size<L>; ++i)
        if ( flat_index((L&&)l,i) != flat_index((R&&)r,i) )
          return flat_index((L&&)l,i) < flat_index((R&&)r,i);
      return false;
    }
  }

  template <typename A>
  constexpr bool operator()(A const& l, A const& r) const noexcept(
    noexcept(operator()<A const&, A const&>(l,r)))
  {
    return operator()<A const&, A const&>(l,r);
  }

  using is_transparent = void;
};

#undef UINTPTR

namespace impl {

template <typename T>
struct member_default_3way_check
{
  T v;
  auto operator<=>(member_default_3way_check const&) const = default;
};

template <typename T>
struct member_default_equality_check
{
  T v;
  bool operator==(member_default_equality_check const&) const = default;
};

// detect gcc bug 93480
inline constexpr bool GCC10_ARRAY_COMPARE_WORKAROUND = []{
  struct A { int v[1]; bool operator==(A const&) const = default; };
  return A{} != A{}; // evaluates true with gcc bug #93480
}();

} // impl

// member_default_3way
//   trait to check if member of type T can have defaulted <=>
//
template <typename T>
inline constexpr bool member_default_3way = []
{
  if constexpr (is_array_v<T> && impl::GCC10_ARRAY_COMPARE_WORKAROUND)
    return false;
  else
    return std::three_way_comparable<impl::member_default_3way_check<T>>;
}();

// member_default_equality
//   trait to check if member of type T can have defaulted ==
//
template <typename T>
inline constexpr bool member_default_equality = []
{
  if constexpr (is_array_v<T> && impl::GCC10_ARRAY_COMPARE_WORKAROUND)
    return false;
  else
    return std::equality_comparable<impl::member_default_equality_check<T>>;
}();

#include "namespace.hpp"

#endif // LTL_ARRAY_COMPARE_HPP
