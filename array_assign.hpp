//  Copyright (c) 2021 Will Wray https://keybase.io/willwray
//
//  Distributed under the Boost Software License, Version 1.0.
//        http://www.boost.org/LICENSE_1_0.txt
//
//  Repo: https://github.com/willwray/c_array_support

#ifndef LTL_ARRAY_ASSIGN_HPP
#define LTL_ARRAY_ASSIGN_HPP
/*
  array_assign.hpp
 -=================-

  Concepts and functor for generic assignment including C-array.

  Avoids <algorithm> or <functional> dependency; implements algorithms
  equivalent to std::copy, for possibly nested arrays (currently lacks
  optimizations such as memcpy specializations).

  Concepts:

    ltl::assignable_from         c.f. std::assignable_from
    ltl::default_assignable      (no std equivalent)

    default_assignable<T> captures language concept that a value can be
    assigned from an empty braced initializer list: v = {};

    It's implied by default_initializable<T> && assignable_from<T&, T>.
    Class types can also model the concept with an operator=({}).

  Functors:
    ltl::assign

  Usage
  =====

    template <ltl::default_assignable...T>
    void clear(T&&...v) { (ltl::assign{}((T&&)v), ...); }

    template <typename L, typename R>
      requires ltl::assignable_from<L&,R&&>
    L& ass(L& l, R&& r) {
      return ltl::assign{}(l,(R&&)r);
    }
  
  See the ltl::tupl implementation for its use in array member support.

  Raison d'etre
  =============
   C-array doesn't have copy semantics; in particular assignment:

      int a[2];
      int b[2]{};
      a = b;    // error: array type 'int [2]' is not assignable
      a = {};   // error: assigning to an array from an initializer list
      a = {1,2};

   P1997 fixes this. In the meantime, C++20 doesn't have std facilities
   for generic assignment, as for comparisons (see array_compare.hpp),
   so this header attempts a similar 'polyfill'.
*/

#include <concepts>

#include "c_array_support.hpp"

#include "namespace.hpp"

template <typename T>
concept default_assignable =
          requires(flat_element_t<T> v) { (T)v={}; };

template <typename L, typename R>
concept assignable_from =
   std::assignable_from< flat_element_t<L>, flat_element_t<R> >
&& same_extents_v<std::remove_cvref_t<L>,std::remove_cvref_t<R>>;

// assign functor, includes array assignment unlike core language op=
// assign(l) single-argument form is default-assign ={} (element-wise)
template <typename L = void> struct assign_to {};

template <typename L> concept assign_toable = requires {typename assign_to<L>::value_type; };

template <c_array L> struct assign_to<L>
{
    L& l;

    using value_type = std::remove_reference_t<L>;

    constexpr assign_to const operator=(assign_to<>) const
        noexcept(noexcept(flat_index(l) = {}))
        requires default_assignable<L&>
    {
        for (int i = 0; i != flat_size<L>; ++i)
            flat_index(l, i) = {};
        return *this;
    }

    template <typename R>
    requires assignable_from<L, R&&>
        && same_extents_v<value_type, std::remove_cvref_t<R>>
        constexpr assign_to const operator=(R&& r) const
        noexcept(noexcept(flat_index(l) = flat_index((R&&)r)))
    {
        for (int i = 0; i != flat_size<L>; ++i)
            flat_index(l, i) = flat_index((R&&)r, i);
        return *this;
    }

    template <int N>
    requires (N == std::extent_v<value_type>)
        constexpr assign_to const operator=(std::remove_extent_t<value_type>const(&r)[N]) const
        noexcept(noexcept(flat_index(l) = flat_index(r)))
    {
        return operator=<value_type const&>(r);
    }

};
template <typename L> assign_to(L&&)->assign_to<L&&>;

template <typename T>
std::remove_reference_t<T> const& as_const(T&& v) { return v; }

template <typename L>
decltype(auto) assign(L&& l) {
    if constexpr (assign_toable<L&&>)
        return std::type_identity_t<assign_to<L&&> const>{l};
    else
        return (L&&)l;
}

#include "namespace.hpp"

#endif // LTL_ARRAY_ASSIGN_HPP
