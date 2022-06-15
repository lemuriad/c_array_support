//  Copyright (c) 2021 Will Wray https://keybase.io/willwray
//
//  Distributed under the Boost Software License, Version 1.0.
//        http://www.boost.org/LICENSE_1_0.txt
//
//  Repo: https://github.com/willwray/c_array_support

#ifndef LTL_UTIL_TRAITS_HPP
#define LTL_UTIL_TRAITS_HPP

#include <type_traits>

#include "namespace.hpp"

// apply_ref and copy_ref utilities
//
namespace impl {

template<typename L, typename R>
auto apply_ref()
{
  if constexpr (std::is_reference_v<L>)
  {
    if constexpr (std::is_lvalue_reference_v<L>)
      return std::type_identity<R&>{};
    else
      return std::type_identity<R&&>{};
  }
  else
    return std::type_identity<R>{};
};

template<typename L, typename R>
auto apply_cv()
{
  if constexpr (std::is_const_v<L>)
  {
    if constexpr (std::is_volatile_v<L>)
      return std::type_identity<R const volatile>{};
    else
      return std::type_identity<R const>{};
  }
  else
  {
    if constexpr (std::is_volatile_v<L>)
      return std::type_identity<R volatile>{};
    else
      return std::type_identity<R>{};
  }
}
} // impl

// apply_ref<L,R> applies any reference qualifier on L to R
//             => reference collapse if R is already a reference
template <typename L, typename R>
using apply_ref = typename decltype(impl::apply_ref<L,R>())::type;

// copy_ref<L,R> imposes any reference qualifier on L to R
//
template <typename L, typename R>
using copy_ref = apply_ref<L, std::remove_reference_t<R>>;

// apply_cv<L,R> applies any cv qualifier on L to R
//
template <typename L, typename R>
using apply_cv = typename decltype(impl::apply_cv<L,R>())::type;

// copy_cv<L,R> imposes any cv qualifiers on L to R
//
template <typename L, typename R>
using copy_cv = apply_cv<L, std::remove_cvref_t<R>>;

template <typename L, typename R>
using apply_cvref = apply_ref<L,apply_cv<std::remove_reference_t<L>,R>>;

template <typename L, typename R>
using copy_cvref = apply_cvref<L,std::remove_cvref<R>>;

#include "namespace.hpp"

#endif
