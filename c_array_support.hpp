//  Copyright (c) 2021 Will Wray https://keybase.io/willwray
//
//  Distributed under the Boost Software License, Version 1.0.
//        http://www.boost.org/LICENSE_1_0.txt
//
//  Repo: https://github.com/willwray/c_array_support

#ifndef LTL_C_ARRAY_SUPPORT_HPP
#define LTL_C_ARRAY_SUPPORT_HPP

/* Concepts, traits and utils for handing C arrays, possibly nested. */

#include <type_traits>

#include "namespace.hpp"

// c_array concept: matches C array, including references to C array
template <typename A>
concept c_array = std::is_array_v<std::remove_cvref_t<A>>;

// flat_size<A>: total number of elements in A
//               the product of extents of all ranks of A
template <c_array Ar>
inline constexpr auto flat_size = [] {
    using A = std::remove_cvref_t<Ar>;
    if      constexpr (std::rank_v<A> == 1)
              return std::extent_v<A>;
    else if constexpr (std::rank_v<A> == 2)
              return std::extent_v<A,0> * std::extent_v<A,1>;
    else      return std::extent_v<A,0> * std::extent_v<A,1>
           * flat_size<std::remove_extent_t<std::remove_extent_t<A>>>;
}();

// c_array_unpadded concept: matches C arrays (and refs) with no padding
template <typename A>
concept c_array_unpadded =
       c_array<A>
    && sizeof(A) == flat_size<A> * sizeof(std::remove_all_extents_t<
                                               std::remove_cvref_t<A>>);
// c_array_t<T,I...> is an alias to array type T[I][...]
namespace impl { template <typename T, int...> extern T c_array_tv; }
//
template <typename T, int... I>
using c_array_t = decltype(impl::c_array_tv<T,I...>);
//
namespace impl {
template <typename T, int J, int...I>
extern c_array_t<T,I...> c_array_tv<T,J,I...>[J];

template<class A>extern std::remove_all_extents_t<A> flat_elem_t;
template<class A>extern std::remove_all_extents_t<A>& flat_elem_t<A&>;
template<class A>extern std::remove_all_extents_t<A>&& flat_elem_t<A&&>;

template<class A, class E>extern c_array_t<E,flat_size<A>> flat_cast_t;
template<class A, class E>extern c_array_t<E,flat_size<A>>& flat_cast_t<A&,E>;
template<class A, class E>extern c_array_t<E,flat_size<A>>&& flat_cast_t<A&&,E>;
}

// flat_element_t<T> remove_all_extents, under any reference qual
template <typename T>
using flat_element_t = decltype(impl::flat_elem_t<T>);

// flat_cast_t<A>: the type of the flattened array A
template <c_array_unpadded A, typename E = std::remove_all_extents_t<
                                            std::remove_reference_t<A>>>
using flat_cast_t = decltype(impl::flat_cast_t<A,E>);

// same_extents_v<A,B>: trait to tell if A and B have the same extents;
// array types with the same extents or both rank 0 (non-array) types
template <typename A, typename B>
inline constexpr bool same_extents_v =
      (std::rank_v<A> == 0 && std::rank_v<B> == 0)
   || (std::extent_v<A> == std::extent_v<B>
    && same_extents_v<std::remove_extent_t<A>,std::remove_extent_t<B>>);

// subscript(a,i): returns a[i], an rvalue if arg 'a' is an array rvalue
//  workaround for MSVC https://developercommunity.visualstudio.com/t/
//  subscript-expression-with-an-rvalue-array-is-an-xv/1317259
template <c_array A>
constexpr auto&& subscript(A&& a, std::size_t i = 0) noexcept {
    if constexpr (std::is_reference_v<A>)
        return a[i];
    else
        return static_cast<std::remove_extent_t<A>&&>(a[i]);
}

#if defined (MSC_VER)
#define SUBSCRIP(a,i) subscript(a,i)
#else
#define SUBSCRIP(a,i) (a)[i]
#endif

// flat_index(arg) -> arg; requires non-array arg, the identity function
template <typename A> requires (! c_array<A>)
constexpr auto&& flat_index(A&& a) noexcept
      { return static_cast<A&&>(a); }

// flat_index(arg,i) -> arg[?][?...]; requires C array arg, returns a[i]
// the element at index i of the fully flattened array
template <c_array Ar>
constexpr auto&& flat_index(Ar&& a, std::size_t i = 0) noexcept
{
    using A = std::remove_reference_t<Ar>;
    using E = std::remove_all_extents_t<A>;
    using R = std::conditional_t<std::is_reference_v<Ar>,E&,E&&>;
    if (std::is_constant_evaluated() || ! c_array_unpadded<A>)
    {
        if constexpr (std::rank_v<A> == 1)
            return (R)SUBSCRIP(static_cast<Ar&&>(a), i);
        else {
            constexpr auto N = flat_size<decltype(a[0])>;
            return (R)flat_index( SUBSCRIP(static_cast<Ar&&>(a),i/N), i%N);
        }
    }
    else
        return (R)SUBSCRIP(reinterpret_cast<flat_cast_t<Ar&&>>(a),i);
}

#undef SUBSCRIP

#include "namespace.hpp"

#endif
