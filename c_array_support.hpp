//  Copyright (c) 2021 Will Wray https://keybase.io/willwray
//
//  Distributed under the Boost Software License, Version 1.0.
//        http://www.boost.org/LICENSE_1_0.txt
//
//  Repo: https://github.com/willwray/c_array_support

#ifndef LTL_C_ARRAY_SUPPORT_HPP
#define LTL_C_ARRAY_SUPPORT_HPP

/* Concepts, traits and utils for handing C arrays, possibly nested. */

/* Zero-size array, T[0], is supported as far as possible.
   Illegal in standard C++, implementations have varied support for T[0].
   Zero-size array as a final struct member is old and widely supported.
   Some std::traits fail or give erroneous results for T[0].
   Elsewhere, some -pedantic warnings can be elevated to errors.     */

/*
 Value traits:
 - is_array_v<A>: c.f. std::is_array_v, zero-size-array accepting
 - flat_size<A>:  total number of elements in flattened array A
 - rank_v<A>:     c.f. std::rank_v, zero-size-array accepting

 Concepts:
 - bounded_array<A>:    bounded array type, zero-size-array accepting
 - c_array<A>: concept, matches C array, including references to C array
 - c_array_unpadded<A>: concept matches C array & refs, with no padding

 Type traits:
 - remove_extent_t<A>:   c.f. std::remove_extent_t, zero-size-array
 - remove_all_extents_t<A>:   std::remove_all_extents_t, zero-size-array

 - c_array_t<T,I...>: template alias to C array type T[I][...]

 - subscript_t<A>:    remove_extent, under any reference qual
 - flat_element_t<T>: remove_all_extents, under any reference qual
 - flat_cast_t<A>:    The type of the flattened array A, preserving qual

Functions:
 - subscript(a,i):    returns a[i], an rvalue if 'a' is an rvalue
 - flat_index(a,i=0): returns element at i in flat_cast_t<decltype(a)>
*/

#include <type_traits>

#include "namespace.hpp"

// ALLOW_ZERO_SIZE_ARRAY(...): silence -pedantic warnings in ... block
#if defined(__clang__) || defined(__GNUG__)
#   define ALLOW_ZERO_SIZE_ARRAY(...) \
_Pragma("GCC diagnostic push") \
_Pragma("GCC diagnostic ignored \"-Wpedantic\"") \
__VA_ARGS__ \
_Pragma("GCC diagnostic pop")
#else
#   define ALLOW_ZERO_SIZE_ARRAY(...) \
__pragma(warning(push)) \
__pragma(warning(disable:6246)) \
__VA_ARGS__ \
__pragma(warning(pop))
#endif

inline namespace move {
template <typename T>
constexpr std::remove_reference_t<T>&& mv( T&& v ) noexcept
  { return static_cast<std::remove_reference_t<T>&&>(v); }
}
// is_array_v<A>: zero-size-array accepting version of std::is_array_v
template <typename A>
inline constexpr bool is_array_v = false;

template <typename A>
   requires std::is_object_v<A>
&& requires (A p) { requires ! std::is_same_v<A, decltype(p)>; }
inline constexpr bool is_array_v<A> = true;

// bounded_array<A>: a bounded array type, zero-size-array included
template <typename A>
concept bounded_array = is_array_v<A>
    && ! std::is_unbounded_array_v<A>;

// c_array concept: matches C array, including references to C array
// zero-size array included (but not unbounded / incomplete array type)
template <typename A>
concept c_array = bounded_array<std::remove_cvref_t<A>>;

// flat_size<A>: total number of elements in A
//               the product of extents of all ranks of A
template <c_array Ar>
inline constexpr auto flat_size = [] {
    using A = std::remove_cvref_t<Ar>;
    using E = std::remove_extent_t<A>;
    constexpr auto E0 = std::extent_v<A>;
    if constexpr (E0 == 0 || ! is_array_v<E>)
         return E0;
    else return E0 * flat_size<E>;
}();

template <typename A> requires (! is_array_v<A>)
auto remove_extent_f() -> std::type_identity<A>;

template <typename A> requires is_array_v<A>
auto remove_extent_f(A a={}) -> std::remove_pointer<decltype(a)>;

// remove_extent_t<A>: zero-size-array-robust std::remove_extent_t
template <typename A> using remove_extent_t =
          typename decltype(remove_extent_f<A>())::type;

template <typename A>
struct remove_all_extents { using type = A; };

template <typename A> requires is_array_v<A>
struct remove_all_extents<A> {
    using type = typename remove_all_extents<remove_extent_t<A>>::type;
};

// remove_all_extents_t<A>: zero-size-array std::remove_all_extents_t
template <typename A>
using remove_all_extents_t = typename remove_all_extents<A>::type;

// rank_v<A>: zero-size-array accepting version of std::rank_v
template <typename A>
inline constexpr std::size_t rank_v = [] {
    if constexpr (is_array_v<A>)
         return std::size_t{1} + rank_v<remove_extent_t<A>>;
    else return std::size_t{0};
}();

// c_array_unpadded concept: matches C arrays (and refs) with no padding
template <typename A>
concept c_array_unpadded = c_array<A>
    && (flat_size<A> == 0
     || flat_size<A> * sizeof(std::remove_all_extents_t<
                              std::remove_cvref_t<A>>) == sizeof(A));
//
namespace impl {
template <typename T, int...> extern T c_array_tv;
}
//
// c_array_t<T,I...> is an alias to array type T[I][...]
template <typename T, int... I>
using c_array_t = decltype(impl::c_array_tv<T,I...>);
//
namespace impl {
ALLOW_ZERO_SIZE_ARRAY(
template <typename T, int J, int...I>
extern c_array_t<T, I...> c_array_tv<T, J, I...>[J];
)
template<typename RQ, typename T>
using copy_ref = std::conditional_t<
                 std::is_lvalue_reference_v<RQ>, T&, T&&>;

template<typename Q, typename T>
using copy_qual = std::conditional_t<
                 std::is_reference_v<Q>, copy_ref<Q,T>, T>;
}

// subscript_t<T> remove_extent, under any reference qual
template <c_array A>
using subscript_t = impl::copy_qual<A,std::remove_extent_t<
                                      std::remove_reference_t<A>>>;

// flat_element_t<T> remove_all_extents, under any reference qual
template <typename T>
using flat_element_t = impl::copy_qual<T,std::remove_all_extents_t<
                                         std::remove_reference_t<T>>>;

// flat_cast_t<A>: the type of the flattened array A
template <c_array_unpadded A, typename E = std::remove_all_extents_t<
                                            std::remove_reference_t<A>>>
using flat_cast_t = impl::copy_qual<A, E[flat_size<A>]>;

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
constexpr auto&& subscript(A&& a, std::size_t i = 0) noexcept
{
    if constexpr (std::is_reference_v<A>)
        return a[i];
    else
        return mv(a[i]);
}

template <typename E, int N>
constexpr auto& flat_index_recurse(E(&a)[N], std::size_t i = 0) noexcept
{
    if constexpr (std::rank_v<E> == 0)
        return a[i];
    else {
        constexpr auto M = flat_size<E>;
        return flat_index_recurse(a[i/M], i%M);
    }
}

// flat_index(arg,i) requires C array arg, returns a[i]
// the element at index i of the fully flattened array
template <c_array Ar>
constexpr auto flat_index(Ar&& a, std::size_t i = 0) noexcept
        -> flat_element_t<Ar&&>
{
    if (std::is_constant_evaluated() || ! c_array_unpadded<Ar>)
        return static_cast<flat_element_t<Ar&&>>(
               flat_index_recurse(a,i));
    else
        return static_cast<flat_element_t<Ar&&>>(
               reinterpret_cast<flat_cast_t<Ar&&>>(a)[i]);
}

// flat_index(arg) -> arg; requires non-array arg, the identity function
template <typename A> requires (! c_array<A>)
constexpr auto&& flat_index(A&& a) noexcept
      { return static_cast<A&&>(a); }

#include "namespace.hpp"

#endif
