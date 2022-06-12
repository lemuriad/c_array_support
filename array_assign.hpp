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
  ================

  Concepts, class template and function for making assignment generic,
  primarily to facilitate C array assignment (until such a time that C++
  language rules are relaxed to allow more array copy semantics; P1997).

  Avoids <algorithm> or <functional> dependency; implements algorithms
  equivalent to std::ranges::copy, for possibly nested arrays (currently
  lacks optimizations such as memcpy specializations).

  Generic assignment can also be used to define extensible assignment
  outside of class scope (as operator= has to be a member function).

 Concepts:

   ltl::assignable_from         c.f. std::assignable_from
   ltl::default_assignable      see below (no std equivalent)
   ltl::const_assignable        see below (no std equivalent)
   ltl::assign_toable           see below (no std equivalent)

   default_assignable<T> captures the language concept that a value can
   be assigned from an empty braced initializer list; v = {}
   as implied by default_initializable<T> && assignable_from<T&, T>,
   though class types can also model the concept with an operator=({}).

 Traits:
   ltl::is_copy_assignable_v    c.f. std::is_copy_assignable_v
   ltl::is_move_assignable_v    c.f. std::is_move_assignable_v

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
   ltl::assign(a) = {1,2}; // A reference-wrapper for assignment
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

  C array doesn't have copy semantics; in particular assignment:

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

// assignable_from<L,R> version of std::assignable_from, true for arrays
//
template <typename L, typename R>
concept assignable_from =
   std::assignable_from< all_extents_removed_t<L>,
                         all_extents_removed_t<R> >
   && same_extents<std::remove_cvref_t<L>,
                   std::remove_cvref_t<R>>;

// is_copy_assignable_v<T> array version of std::is_copy_assignable_v
//
template <typename T>
inline constexpr bool is_copy_assignable_v
               = std::is_copy_assignable_v<all_extents_removed_t<T>>;

// is_move_assignable_v<T> array version of std::is_move_assignable_v
//
template <typename T>
inline constexpr bool is_move_assignable_v
               = std::is_move_assignable_v<all_extents_removed_t<T>>;

// default_assignable<T> can be assigned from an empty braced init-list
// A new concept, defined true for array
//
template <typename T>
concept default_assignable =
  requires (all_extents_removed_t<T> v) {
    static_cast<all_extents_removed_t<T>>(v) = {};
  };

// const_assignable<T> captures the property of references and proxies
// that they can be assigned to even if const qualified.
//
template <typename T> concept const_assignable =
      requires (all_extents_removed_t<T> const c) { c=c; };
//
template <typename T> using is_const_assignable =
            std::bool_constant<const_assignable<T>>;

// assign_to functor, customization point
// invoked by assign(l) function for assign_toable types
//
template <typename L> struct assign_to {};
template <typename L> assign_to(L&&) -> assign_to<L&&>;

template <typename L> concept assign_toable =
  requires { typename assign_to<L>::value_type; };

// assign_to specialization for array assignment
// (note: operator=(R) -> L& always returns unwrapped type, not wrapper)
//
template <c_array L>
struct assign_to<L>
{
  L& l;

  using value_type = std::remove_reference_t<L>;
  using remove_extent_t = std::remove_extent_t<value_type>;

  // operator=({}) overload for emtpy braced-init
  //
  constexpr L& operator=(assign_to<void>) const
      noexcept(noexcept(flat_index(l) = {}))
    requires default_assignable<L&>
  {
      for (int i = 0; i != flat_size<L>; ++i)
          flat_index(l, i) = {};
      return l;
  }

  // operator=(lval) overload for array lvalues (and rvalue variables)
  //
  template <c_array R>
    requires assignable_from<L, R&&>
  constexpr L& operator=(R&& r) const
      noexcept(noexcept(flat_index(l) = flat_index((R&&)r)))
  {
      for (int i = 0; i != flat_size<L>; ++i)
          flat_index(l, i) = flat_index((R&&)r, i);
      return l;
  }

  // operator=(rval) overload for array rvalue from braced-init
  //
  constexpr L& operator=(value_type const& r) const
      noexcept(noexcept(flat_index(l) = flat_index(r)))
  {
      return operator=<value_type const&>(r);
  }

};

// assign(l) returns assign_to{l}, if assign_toable, else reference-to-l
template <typename L>
constexpr decltype(auto) assign(L&& l) noexcept
{
    if constexpr (assign_toable<L&&>)
        return std::type_identity_t<assign_to<L&&> const>{l};
    else
        return (L&&)l;
}

template <typename L, typename R>
  requires (std::assignable_from<L&,R&&> || assign_toable<L&&>)
constexpr decltype(auto) assign(L&& l, R&& r)
  noexcept(noexcept(assign(l)=(R&&)r))
{
    if constexpr (assign_toable<L&&>)
        return std::type_identity_t<assign_to<L&&> const>{l} = (R&&)r;
    else
        return l = (R&&)r;
}

template <typename L>
constexpr decltype(auto) assign(L&& l, std::remove_cvref_t<L> const& r)
  noexcept(noexcept(assign(l)=r))
{
    if constexpr (assign_toable<L&&>)
        return std::type_identity_t<assign_to<L&&> const>{l} = r;
    else {
        static_assert(std::assignable_from<L&,L const&>);
        return l = r;
    }
}

#include "namespace.hpp"

#endif // LTL_ARRAY_ASSIGN_HPP
