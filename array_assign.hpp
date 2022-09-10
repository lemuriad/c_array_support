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

  Function assign(a,b) provides a uniform way to assign to unknown types
  in generic code, working for arrays as well as assignable types.

  It's a 'polyfill' until C++ adds array copy semantics to the language
  as proposed in C++ wg21.link:/P1997 "Relaxing restrictions on array".
  It automatically detects such language support and switches to use it.

  It requires C++20 and depends on <concepts> and "c_array_support.hpp".

  Problem
  =======
  Assignment of C arrays is ill-formed today, e.g. given  int a[2]{};

    int b[2] = a; // error: array must be initialized with a braced init
    a = b;        // error: array type 'int [2]' is not assignable
    a = {1,2};    // error: assigning to an array from an init-list

  Sadly, array copy initialization is out of reach of library solution
  (despite special cases: auto c=[a]{}; auto [a0,a1]=a; char s[]{"s"};),
  as is array by-value function argument and return type.

  Array assignment, on the other hand, is straightforward to implement
  but operator= can't be overloaded for array so can't be used directly.

  Usage
  =====
    ltl::assign(a,b)     or  ltl::assign(a) = b
    ltl::assign(a,{1,2}) or  ltl::assign(a) = {1,2}
    ltl::assign(a,{})    or  ltl::assign(a) = {}   // e={} forall e in a

  An lvalue reference to a is returned, as for a=b. To constrain assign
  usage, use polyfill versions of std assignability traits and concepts:

   ltl::is_copy_assignable_v<T> = std::is_copy_assignable_v<E>
   ltl::is_move_assignable_v<T> = std::is_move_assignable_v<E>
   ltl::assignable_from<L,R> = std::assignable_from<EL,ER>
                               (&& L and R have the same extents)

  They check element type E = ltl::all_extents_removed<T> instead of T.
  A new concept is introduced to capture assignment from empty braces:

   ltl::default_assignable<T> true if E v{}; v={}; is well formed
                              similar to (std::default_initializable<E>
                                          && std::assignable_from<E&,E>)
  assign_to
  =========
  The actual array-array assignment is done by an 'assign_to' reference-
  wrapper 'proxy', specialized here only for assign_to<c_array>.

   ltl::assign_to{a} = b; // calls assign_to<int[2]>::operator=(b)
   ltl::assign_to<T> Customize by specializing with operator= overloads,
                     & define a value_type alias for detection purposes.
   ltl::assign_toable<T> true if T has typename assign_to<T>::value_type

   ltl::assign(l) returns assign_to<L>, if extant, else a reference to l
   ltl::assign(l,r) delegates to assign_to<L> if it is specialized for L
                    else for assignable types just does assignment l = r
  Performance
  ===========
  Copy optimizations such as memcpy specializations are not done, yet.
  Nested array copies have constexpr and runtime implementations.
*/

#include <concepts>

#include "c_array_support.hpp"

#include "namespace.hpp"

// assignable_from<L,R> array-enabled version of std::assignable_from
//
template <typename L, typename R>
concept assignable_from =
   std::assignable_from<all_extents_removed_t<L>,
                        all_extents_removed_t<R>>
   && same_extents<std::remove_cvref_t<L>,
                   std::remove_cvref_t<R>>;

// is_copy_assignable_v<T> array version of std::is_copy_assignable_v
//
template <typename T>
inline constexpr bool is_copy_assignable_v
               = std::is_copy_assignable_v<all_extents_removed_t<T>>;

template <typename T> using is_copy_assignable
       = std::bool_constant<is_copy_assignable_v<T>>;

// is_move_assignable_v<T> array version of std::is_move_assignable_v
//
template <typename T>
inline constexpr bool is_move_assignable_v
               = std::is_move_assignable_v<all_extents_removed_t<T>>;

template <typename T> using is_move_assignable
       = std::bool_constant<is_move_assignable_v<T>>;

// default_assignable<T> can be assigned from an empty braced init-list
// Defined true for array of default_assignable element type.
//
template <typename T>
concept default_assignable =
  requires (all_extents_removed_t<T> v) {
    static_cast<all_extents_removed_t<T>>(v) = {};
  };

// assign_to functor, customization point
// invoked by assign(l) function for assign_toable types
//
template <typename L> struct assign_to;
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

  // operator=({}) overload for emtpy braced-init
  //
  constexpr L& operator=(std::true_type) const
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
        return std::add_const_t<assign_to<L&&>>{l};
    else
        return (L&&)l;
}

template <typename L, typename R>
  requires (std::assignable_from<L&,R&&> || assign_toable<L&&>)
constexpr decltype(auto) assign(L&& l, R&& r)
  noexcept(noexcept(assign(l)=(R&&)r))
{
    if constexpr (assign_toable<L&&>)
        return std::add_const_t<assign_to<L&&>>{l} = (R&&)r;
    else
        return l = (R&&)r;
}

template <typename L>
constexpr decltype(auto) assign(L&& l, std::remove_cvref_t<L> const& r)
  noexcept(noexcept(assign(l)=r))
{
    if constexpr (assign_toable<L&&>)
        return std::add_const_t<assign_to<L&&>>{l} = r;
    else {
        static_assert(std::assignable_from<L&,L const&>);
        return l = r;
    }
}

#include "namespace.hpp"

#endif // LTL_ARRAY_ASSIGN_HPP
