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

  Concepts, class template and function for making assignment generic,
  primarily to facilitate C-array assignment (until such a time that C++
  language rules are relaxed to allow more array copy semantics).

  Avoids <algorithm> or <functional> dependency; implements algorithms
  equivalent to std::copy, for possibly nested arrays (currently lacks
  optimizations such as memcpy specializations).

  Generic assignment is also useful for defining extended assignment
  outside of class scope (as operator=(R) has to be a member function).
  For example, tuple assignment allows implicit conversions of elements,
  and has many special assignments which bloat its API and are better
  defined externally with a means for extension by the user.

  Concepts:

    ltl::assignable_from         c.f. std::assignable_from
    ltl::default_assignable      see below (no std equivalent)
    ltl::assign_toable           see below (no std equivalent)

    default_assignable<T> captures language concept that a value can be
    'zero-init' assigned from an empty braced initializer list: v = {};
    as implied by default_initializable<T> && assignable_from<T&, T>,
    though class types can also model the concept with an operator=({}).

  Class template:
    ltl::assign_to<T> customization point for users to specialize,
                      with operator=(R) overloads
  Function:
    ltl::assign(l,r) assign r to l
                 (r may be a braced init list, empty for default-init)

    ltl::assign(l) return assign_to<L> if extant else return l reference

  Usage
  =====

    int a[2], b[2];
    ltl::assign(a) = {1,2}; // A reference-wrapper fpr assignment
    ltl::assign(b,{3,4});
    ltl::assign_to{a} = b;
    ltl::assign(a) = {};

    template <ltl::default_assignable...T>
    void clear(T&&...v) { (ltl::assign((T&&)v,{}), ...); }

    template <typename L, typename R>
      requires ltl::assignable_from<L&,R&&>
    L& ass(L& l, R&& r) { return ltl::assign(l,(R&&)r); }
  
  See the ltl::tupl implementation for its use in array member support.

  Raison d'etre
  =============
  This header is a 'polyfill' for lack of generic assignment support,
  in the language or standard library.

  C-array doesn't have copy semantics; in particular assignment:

      int a[2];
      int b[2]{};
      a = b;    // error: array type 'int [2]' is not assignable
      a = {};   // error: assigning to an array from an initializer list
      a = {1,2}; // ""

   P1997 fixes this. In the meantime, C++20 doesn't have std facilities
   for generic assignment, as it has for comparisons.
*/

#include <concepts>

#include "c_array_support.hpp"

#include "namespace.hpp"

template <typename T>
concept default_assignable =
  requires (flat_element_t<T> v) {
    static_cast<flat_element_t<T>>(v) = {};
  };

template <typename L, typename R>
concept assignable_from =
   std::assignable_from< flat_element_t<L>, flat_element_t<R> >
&& same_extents_v<std::remove_cvref_t<L>,std::remove_cvref_t<R>>;

// assign_to functor, customization point
// invoked by assign(l) function for assign_toable types
template <typename L> struct assign_to {};
template <typename L> assign_to(L&&) -> assign_to<L&&>;

template <typename L> concept assign_toable =
  requires { typename assign_to<L>::value_type; };

// assign_to specialization for array assignment
// (note: operator=(R) -> L& always returns wrapped type, not wrapper)
template <c_array L>
struct assign_to<L>
{
  L& l;

  using value_type = std::remove_reference_t<L>;
  using remove_extent_t = std::remove_extent_t<value_type>;

  // operator=({}) overload for emtpy braced-init
  constexpr L& operator=(assign_to<void>) const
      noexcept(noexcept(flat_index(l) = {}))
    requires default_assignable<L&>
  {
      for (int i = 0; i != flat_size<L>; ++i)
          flat_index(l, i) = {};
      return l;
  }

  // operator=(lval) overload for array lvalues (and rvalue variables)
  template <c_array R>
    requires assignable_from<L, R&&>
          && same_extents_v<value_type, std::remove_cvref_t<R>>
  constexpr L& operator=(R&& r) const
      noexcept(noexcept(flat_index(l) = flat_index((R&&)r)))
  {
      for (int i = 0; i != flat_size<L>; ++i)
          flat_index(l, i) = flat_index((R&&)r, i);
      return l;
  }

  // operator=(rval) overload for array rvalue from braced-init
  template <int N>
    requires (N == std::extent_v<value_type>)
  constexpr L& operator=(remove_extent_t const(&r)[N]) const
      noexcept(noexcept(flat_index(l) = flat_index(r)))
  {
      return operator=<value_type const&>(r);
  }

};

// assign(l) returns assign_to{l}, if assign_toable, else reference-to-l
template <typename L>
decltype(auto) assign(L&& l) {
    if constexpr (assign_toable<L&&>)
        return std::type_identity_t<assign_to<L&&> const>{l};
    else
        return (L&&)l;
}

template <typename L, typename R>
decltype(auto) assign(L&& l, R&& r) {
    if constexpr (assign_toable<L&&>)
        return std::type_identity_t<assign_to<L&&> const>{l} = (R&&)r;
    else {
        static_assert(std::assignable_from<L&,R&&>);
        return l = (R&&)r;
    }
}

template <typename L>
decltype(auto) assign(L&& l, std::remove_cvref_t<L> const& r) {
    if constexpr (assign_toable<L&&>)
        return std::type_identity_t<assign_to<L&&> const>{l} = r;
    else {
        static_assert(std::assignable_from<L&,L const&>);
        return l = r;
    }
}

template <typename E, int N>
decltype(auto) assign(E(&l)[N], E const(&r)[N]) {
    return std::type_identity_t<assign_to<E(&)[N]> const>{l} = r;
}

#include "namespace.hpp"

#endif // LTL_ARRAY_ASSIGN_HPP
