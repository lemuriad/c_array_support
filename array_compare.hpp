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
 -=================-

   C-array supporting C++20 comparison concepts, aliases and functors.
   (plus some detection traits)

   Depends only on <compare> for three-way operator <=> comparisons.

  Avoids <algorithm> or <functional> dependency; implements algorithms
  equivalent to std::lexicographical_compare_three_way, ranges equality,
  etc. tailored to C array with recursion for possibly nested  arrays
  (lacks optimizations such as memcmp specializations).

  Concepts:

    ltl::three_way_comparable[_with]  c.f. std::three_way_comparable
    ltl::equality_comparable[_with]   c.f. std::equality_comparable
    ltl::totally_ordered[_with]       c.f. std::totally_ordered

    ltl::pointer_equality_comparable_with  (no std equivalent)
    ltl::pointer_less_than_comparable_with (no std equivalent)

  Aliases:
    ltl::compare_three_way_result_t c.f. std::compare_three_way_result_t

  Functors:
    ltl::compare_three_way     c.f. std::compare_three_way
    ltl::equal_to              c.f. std::ranges::equal_to

  Usage
  =====
  Usage is analogous to the std versions. The ltl functors also accept
  rvalue array braced-initializer list as the right hand argument:

      int a22[2][2] {{0,1},{2,3}};
      ltl::compare_three_way{}( a22, {{0,1},{2,2}} ) > 0);

  See the ltl::tupl implementation for its use in array member support.

  Raison d'etre
  =============
  C-array is not comparable, on its own, and C++20 doesn't go far enough
  to fix array comparison or to assist generic code in comparing arrays.

  C++20 introduces operator<=> for three way comparison and allows
  classes to default <=> and/or == including for array members.

  The std::compare_three_way functor defined in <compare> and the ranges
  comparison functors std::ranges::equal_to, etc., do not support array,
  so this header provides lookalikes that support C array.

  (Array-array comparison is deprecated in C++20, could be removed in 23
   then fixed in C++26. Hopefully this header is a stopgap till then.)
*/

#include <compare>

#include "c_array_support.hpp"

#ifndef __UINTPTR_TYPE__
#include <cstdint>
#  define UINTPTR_T uintptr_t
#else
#  define UINTPTR_T __UINTPTR_TYPE__
#endif

#include "namespace.hpp"

// comparable concepts extended to include array type
template <typename A, class Cat = std::partial_ordering>
concept three_way_comparable =
   std::three_way_comparable<std::remove_all_extents_t<
                             std::remove_reference_t<A>>, Cat>;

template <typename L, typename R, class Cat = std::partial_ordering>
concept three_way_comparable_with =
   std::three_way_comparable_with<
     std::remove_all_extents_t<std::remove_reference_t<L>>,
     std::remove_all_extents_t<std::remove_reference_t<R>>, Cat>
  && same_extents_v<std::remove_cvref_t<L>,std::remove_cvref_t<R>>;

template <typename A>
concept equality_comparable =
   std::equality_comparable<std::remove_all_extents_t<
                            std::remove_reference_t<A>>>;

template <typename L, typename R>
concept equality_comparable_with =
   std::equality_comparable_with<
     std::remove_all_extents_t<std::remove_reference_t<L>>,
     std::remove_all_extents_t<std::remove_reference_t<R>>>
  && same_extents_v<std::remove_cvref_t<L>,std::remove_cvref_t<R>>;

template <typename A>
concept totally_ordered =
   std::totally_ordered<std::remove_all_extents_t<
                        std::remove_reference_t<A>>>;

template <typename L, typename R>
concept totally_ordered_with =
   std::totally_ordered_with<
     std::remove_all_extents_t<std::remove_reference_t<L>>,
     std::remove_all_extents_t<std::remove_reference_t<R>>>
  && same_extents_v<std::remove_cvref_t<L>,std::remove_cvref_t<R>>;

template <typename L, typename R>
using compare_three_way_result_t = 
 std::compare_three_way_result_t<
       std::remove_all_extents_t<std::remove_reference_t<L>>,
       std::remove_all_extents_t<std::remove_reference_t<R>>>;

// pointer_equality_comparable_with (NOT extended to array type)
template <typename P, typename Q>
concept pointer_equality_comparable_with =
  ! c_array<std::remove_cvref_t<P>> && ! c_array<std::remove_cvref_t<Q>>
  && requires (P&& l, Q&& r) { {l == r} -> std::same_as<bool>; }
  && std::convertible_to<P, const volatile void*>
  && std::convertible_to<Q, const volatile void*>
  && (! requires(P&& l, Q&& r)
      { operator==(static_cast<P&&>(l), static_cast<Q&&>(r)); }
      &&
      ! requires(P&& l, Q&& r)
      { static_cast<P&&>(l).operator==(static_cast<Q&&>(r)); });

// pointer_less_than_comparable_with (NOT extended to array type)
template <typename P, typename Q>
concept pointer_less_than_comparable_with =
  ! c_array<std::remove_cvref_t<P>> && ! c_array<std::remove_cvref_t<Q>>
  && requires (P&& l, Q&& r) { {l < r} -> std::same_as<bool>; }
  && std::convertible_to<P, const volatile void*>
  && std::convertible_to<Q, const volatile void*>
  && (! requires(P&& l, Q&& r)
        { operator<(static_cast<P&&>(l), static_cast<Q&&>(r)); }
      &&
      ! requires(P&& l, Q&& r)
        { static_cast<P&&>(l).operator<(static_cast<Q&&>(r)); });

// compare_three_way functor extended to compare arrays
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
    else {
        for (int i = 0; i != flat_size<L>; ++i) {
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
  {   return operator()<A const&, A const&>(l,r);   }

  using is_transparent = void;
};

// equal_to functor corrected to compare arrays (rather than array ids)
struct equal_to
{
    template <typename L, typename R>
      requires (equality_comparable_with<L,R>
             || pointer_equality_comparable_with<
                std::remove_all_extents_t<std::remove_reference_t<L>>,
                std::remove_all_extents_t<std::remove_reference_t<R>>>)
    constexpr bool operator()(L&& l, R&& r) const
      noexcept(noexcept(flat_index((L&&)l) == flat_index((R&&)r)))
    {
        if constexpr (! std::is_array_v<std::remove_cvref_t<L>>)
            return (L&&)l == (R&&)r;
        else {
            for (int i = 0; i != flat_size<L>; ++i)
                if ( flat_index((L&&)l,i) != flat_index((R&&)r,i) )
                    return false;
            return true;
        }
    }
    template <typename A>
    constexpr bool operator()(A const& l, A const& r) const noexcept(
      noexcept(operator()<A const&, A const&>(l,r)))
    {   return operator()<A const&, A const&>(l,r);  }

    using is_transparent = void;
};

// not_equal_to functor corrected to compare arrays (rather than array ids)
struct not_equal_to
{
    template <typename L, typename R>
      requires (equality_comparable_with<L,R>
             || pointer_equality_comparable_with<
                std::remove_all_extents_t<std::remove_reference_t<L>>,
                std::remove_all_extents_t<std::remove_reference_t<R>>>)
    constexpr bool operator()(L&& l, R&& r) const
      noexcept(noexcept(flat_index((L&&)l) == flat_index((R&&)r)))
    {
        return ! equal_to{}((L&&)l, (R&&)r);
    }
    template <typename A>
    constexpr bool operator()(A const& l, A const& r) const noexcept(
      noexcept(operator()<A const&, A const&>(l,r)))
    {   return operator()<A const&, A const&>(l,r);  }

    using is_transparent = void;
};


// less functor corrected to compare arrays (rather than array ids)
struct less
{
    template <typename L, typename R>
      requires (totally_ordered_with<L,R>
             || pointer_less_than_comparable_with<
                std::remove_all_extents_t<std::remove_reference_t<L>>,
                std::remove_all_extents_t<std::remove_reference_t<R>>>)
    constexpr bool operator()(L&& l, R&& r) const
      noexcept(noexcept(flat_index((L&&)l) == flat_index((R&&)r)))
    {
      if constexpr (pointer_less_than_comparable_with<L,R>)
      {
          if (std::is_constant_evaluated())
              return l < r;
          return reinterpret_cast<UINTPTR_T>(
                      static_cast<const volatile void*>((L&&)l))
               < reinterpret_cast<UINTPTR_T>(
                      static_cast<const volatile void*>((R&&)r));
      }
      else if constexpr (! std::is_array_v<std::remove_cvref_t<L>>)
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
    {   return operator()<A const&, A const&>(l,r);  }

    using is_transparent = void;
};

#undef UINTPTR_T

// Trait to check if member of type T can have defaulted <=>
template <typename T>
inline constexpr bool member_default_3way = []
{
  struct C { T v; auto operator<=>(C const&) const = default; };
  return std::three_way_comparable<C>;
}();

// Trait to check if member of type T can have defaulted ==
template <typename T>
inline constexpr bool member_default_equality = []
{
  struct C { T v; bool operator==(C const&) const = default; };
  return std::equality_comparable<C>;
}();

#include "namespace.hpp"

#endif // LTL_ARRAY_COMPARE_HPP
