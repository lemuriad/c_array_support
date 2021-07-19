# **`c_array_support`**

## C++20 Concepts, traits and utils for handing C arrays

------------

### Header [`c_array_support.hpp`](#c_array_supporthpp)

### Header [`array_compare.hpp`](#array_comparehpp)

### Header [`array_assign.hpp`](#c_array_assignhpp)

------------

## c_array_support.hpp

Depends on std `<type_traits>` header.

### Concepts

* `ltl::c_array<T>`          matches C array (and references to array)

* `ltl::c_array_unpadded<T>` matches C arrays with no padding (and refs)

The `c_array` concepts also match references to C array,
useful in practice as arrays are passed by reference.
Padding in nested array types is possible but rare. 
Unpadded nested arrays can be safely reinterpret cast to a flat array.

### Aliases

* `ltl::c_array_t<T,N...>` maps variadic Args to array type -> `T[N][...]`

* `ltl::flat_element_t<A>` remove_all_extents, under any reference qualification

* `ltl::flat_cast_t<A>` maps possibly nested array `A` to flattened array type  
  
  E.g. `ltl::flat_cast_t<T[M][N][...]>` -> `T[M*N*...]`

### Traits

* `flat_size<A>` yields the total number of elements in array `A`  
 (the product of extents of all ranks of `A`).

* `same_extents_v<A,B>` predicate to tell if `A` and `B` have the same extents;  
either both array types with the same extents or both rank 0 (non-array) types.

### Functions

* `auto&& flat_index(c_array auto&& a, std::size_t i = 0)`
* `auto&& subscript(c_array auto&& a, std::size_t i = 0)`

`flat_index(arg,i)`returns `a[i]`;
 the element at index `i` of the flattened array.

`subscript(a,i)` returns `a[i]`, an rvalue if the argument is an array rvalue.  
A workaround for MSVC [subscript-expression-with-an-rvalue-array-is-an-xvalue](https://developercommunity.visualstudio.com/t/subscript-expression-with-an-rvalue-array-is-an-xv/1317259)

------------

## array_compare.hpp

C-array supporting comparison concepts, aliases and functors,
mostly replacing std lib features, plus some detection traits.

   Depends only on std `<compare>` for three-way operator <=> support.

* Concepts:

```C++
    ltl::three_way_comparable [_with]  c.f. std::three_way_comparable
    ltl::equality_comparable  [_with]  c.f. std::equality_comparable
    ltl::totally_ordered      [_with]  c.f. std::totally_ordered

    ltl::pointer_equality_comparable_with  (no std equivalent)
    ltl::pointer_less_than_comparable_with (no std equivalent)
```

* Aliases:

```C++
    ltl::compare_three_way_result_t c.f. std::compare_three_way_result_t
```

* Functors:

```C++
    ltl::compare_three_way     c.f. std::compare_three_way
    ltl::equal_to              c.f. std::ranges::equal_to
    ltl::not_equal_to          c.f. std::ranges::not_equal_to
    ltl::less                  c.f. std::ranges::less
```

If you want `greater`, `greater_equal` or `less_equal`  
then cut n paste with:  

```C++
    greater(a,b)       ==   less(b,a)
    greater_equal(a,b) == ! less(a,b)
    less_equal(a,b)    == ! less(b,a)
```

------------

## array_assign.hpp

Depends on std `<concepts>`

### Concepts

* `ltl::default_assignable` (no std equivalent)
* `ltl::assignable_from` (c.f. std)

### Functors

* `ltl::assign` (no std equivalent)
