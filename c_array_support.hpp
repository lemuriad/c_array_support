//  Copyright (c) 2021 Will Wray https://keybase.io/willwray
//
//  Distributed under the Boost Software License, Version 1.0.
//        http://www.boost.org/LICENSE_1_0.txt
//
//  Repo: https://github.com/willwray/c_array_support

#ifndef LTL_C_ARRAY_SUPPORT_HPP
#define LTL_C_ARRAY_SUPPORT_HPP
/*
  c_array_support.hpp
  ===================
 
  c_array concept, type traits and utilities for handing C arrays.

  Depends only on std <type_traits> and uses C++20 features. 

  C arrays are hard to handle, multidimensional arrays especially.
  Generic code ought to handle arrays but they're often neglected.
  If arrays are unloved then zero-size arrays are total outcasts.

  This header provides tools to deal with arrays, possibly nested
  and possibly zero-size, plus corrected versions of some std lib
  traits where current library implementations give wrong results
  for zero-size arrays (it's undefined behavior, so fair enough).

  This is an incomplete collection, including only tools that have
  proven useful so far in practice.

 Alternative versions of std traits that work for T[0]:

  - is_array_v    i.e. ltl::is_array_v replaces std::is_array_v, etc.
  - is_bounded_array_v (more useful in practice than is_array_v)
  - rank_v             (std::extent_v often works by accident)
  - remove_extent_t
  - remove_all_extents
  - remove_all_extents_t

 Concepts:
  - c_array<A>: matches C array, including references to C array
  - c_array_unpadded<A>: matches unpadded C array, including references

 Aliases:
  - c_array_t<T,I...>: template alias to C array type T[I][...]
  - extent_removed_t<A>: remove_extent, under any reference qualifier
  - all_extents_removed_t<T>: remove_all_extents, under any reference qual
  - flat_cast_t<A>: The type of the flattened array A, preserving cvref quals

 Value traits:
  - flat_size<A>: yields the total number of elements in flattened array A
  - same_extents<A,B>: predicate to tell if A and B have the same extents

 Functions:
  - subscript(a,i):    returns a[i], an rvalue if 'a' is an rvalue
  - flat_index(a,i=0): returns element at i in flat_cast_t<decltype(a)>
*/

#include <type_traits>

#include "namespace.hpp"

// c_array_t<T,I...> is an alias to array type T[I][...]
// e.g. c_array_t<int> -> int (not an array if T is not)
//      c_array_t<int,0,2> -> int[0][2]
//
namespace impl {
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
ALLOW_ZERO_SIZE_ARRAY
(
template <typename T, int...>
extern T c_array_tv;
//
template <typename T, int J, int...I>
extern decltype(c_array_tv<T,I...>) c_array_tv<T, J, I...>[J];
)
#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
}// impl
//
template <typename T, int... I>
using c_array_t = decltype(impl::c_array_tv<T,I...>);

// is_array_v<A>
// same as std::is_array_v but supports zero-size array
// (detects array by 'decay' adjustment of a requires-expr parameter
//  after is_object has rejected reference, void and function types)
//
template <typename A>
inline constexpr bool is_array_v = requires {
  requires std::is_object_v<A>;
  requires requires (A p) { requires ! std::is_same_v<A, decltype(p)>; };
};

// is_bounded_array_v<A>
// same as std::is_bounded_array_v but supports zero-size array
//
template <typename A>
inline constexpr bool is_bounded_array_v = is_array_v<A>
          && ! std::is_unbounded_array_v<A>;

// c_array concept: matches C array, including references to C array,
// including zero-size array, but not including unbounded array type
//
template <typename A>
concept c_array = is_bounded_array_v<std::remove_cvref_t<A>>;

// remove_extent_t<A>
namespace impl {
template <typename A> requires (! is_array_v<A>)
auto remove_extent_f() -> std::type_identity<A>;
//
template <typename A> requires is_array_v<A>
auto remove_extent_f(A a={}) -> std::remove_pointer<decltype(a)>;
}
// remove_extent_t<A>
// same as std::remove_extent_t but supports zero-size array
//
template <typename A> using remove_extent_t =
    typename decltype(impl::remove_extent_f<A>())::type;

// remove_all_extents<A>
// same as std::remove_all_extents but supports zero-size array
//
template <typename A>
struct remove_all_extents { using type = A; };
//
template <typename A> requires is_array_v<A>
struct remove_all_extents<A> {
    using type = typename remove_all_extents<remove_extent_t<A>>::type;
};
// remove_all_extents_t<A>
// same as std::remove_all_extents_t but supports zero-size array
//
template <typename A>
using remove_all_extents_t = typename remove_all_extents<A>::type;

// rank_v<A>:
// same as std::rank_v but supports zero-size array
//
template <typename A>
inline constexpr auto rank_v = [] {
    if constexpr (is_array_v<A>)
         return 1 + rank_v<remove_extent_t<A>>; // recurse
    else return 0;
}();

// flat_size<A>: total number of elements in A
//               the product of extents of all ranks of A
//
template <c_array Ar>
inline constexpr auto flat_size = [] {
    using A = std::remove_cvref_t<Ar>;
    using E = remove_extent_t<A>;
    if constexpr (is_array_v<E>)
         return std::extent_v<A> * flat_size<E>; // recurse
    else return std::extent_v<A>;
}();

// c_array_unpadded concept:
// matches C array (and reference-to-array) that has no padding,
// i.e. sizeof array == total element count times sizeof element
// (false on MSVC for zero-size arrays, true on GCC and Clang)
// (are there other cases of 'padded' arrays? Best be paranoid)
//
template <typename A>
concept c_array_unpadded = c_array<A>
    && flat_size<A> * sizeof(remove_all_extents_t<
                             std::remove_cvref_t<A>>) == sizeof(A);

namespace impl {
// copy_ref<R,T> applies any reference qualifier on R to T
//            => reference collapse if T is already a reference
template<typename R, typename T>
using copy_ref = std::conditional_t<std::is_reference_v<R>,
                 std::conditional_t<std::is_lvalue_reference_v<R>,T&,T&&>,T>;
}

// extent_removed_t<T> remove_extent, under any reference qualifier
//                e.g. extent_removed_t<int(&&)[1][2]> -> int(&&)[2]
template <c_array A>
using extent_removed_t = impl::copy_ref<A,remove_extent_t<
                                     std::remove_reference_t<A>>>;

// all_extents_removed_t<T> remove_all_extents, under any reference qualifier
//                     e.g. all_extents_removed_t<int(&)[1][2]> -> int&
template <typename T>
using all_extents_removed_t = impl::copy_ref<T,remove_all_extents_t<
                                          std::remove_reference_t<T>>>;

// flat_cast_t<A>: the type of the flattened array A, preserving cvref quals
//                 e.g. flat_cast_t<int const(&)[2][3]> -> int const(&)[6]
//
template <c_array_unpadded A, typename E = remove_all_extents_t<
                                           std::remove_reference_t<A>>>
using flat_cast_t = impl::copy_ref<A, E[flat_size<A>]>;

// same_extents<A,B>: trait to tell if A and B have the same extents;
// array types with the same extents or both rank 0 (non-array) types
//
template <typename A, typename B>
inline constexpr bool same_extents =
      (std::rank_v<A> == 0 && std::rank_v<B> == 0)
   || (std::extent_v<A> == std::extent_v<B>
    && same_extents<remove_extent_t<A>,remove_extent_t<B>>);

// subscript(a,i):
// returns a[i], an rvalue if argument 'a' is an array rvalue
//  workaround for MSVC https://developercommunity.visualstudio.com/t/
//  subscript-expression-with-an-rvalue-array-is-an-xv/1317259
//
constexpr auto&& subscript(c_array auto&& a, std::size_t i = 0) noexcept
{
  if constexpr (std::is_lvalue_reference_v<decltype(a)>)
    return a[i];
  else {
    auto move = []<typename T>(T&&v)noexcept->std::remove_cvref_t<T>&&
                   {return static_cast<std::remove_cvref_t<T>&&>(v);};
    return move(a[i]);
  }
}

// flat_index_recurse(a,i)
// requires C array a, returns the element at index i of the flattened array.
// A recursive implementation is constexpr-correct but may give poor runtime
// codegen, if it isn't inlined with the div/mod arithmetic folded out.
//
constexpr auto& flat_index_recurse(c_array auto& a, std::size_t i = 0) noexcept
{
  if constexpr (c_array<decltype(a[i])>)
  {
    constexpr auto M = flat_size<decltype(a[i])>;
    return flat_index_recurse(a[i/M], i%M);
  }
  else return a[i];
}

// flat_index(a,i)
// requires C array a, returns the element at index i of the flattened array.
// Non-constant evaluation uses reinterpret_cast for hopefully better codegen.
//
constexpr auto&& flat_index(c_array auto&& a, std::size_t i = 0) noexcept
{
  using A = decltype(a);
  using E = all_extents_removed_t<A>;
  if (std::is_constant_evaluated() || ! c_array_unpadded<A>)
      return static_cast<E>(flat_index_recurse(a,i));
  else
      return static_cast<E>(reinterpret_cast<flat_cast_t<A>>(a)[i]);
}

// flat_index(a) -> a;
// requires non-array a, the identity function for non-array objects
//
template <typename A> requires (! c_array<A>)
constexpr auto&& flat_index(A&& a) noexcept
      { return static_cast<A&&>(a); }

#include "namespace.hpp"

#endif
