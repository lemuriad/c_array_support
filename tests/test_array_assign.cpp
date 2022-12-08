#include "test_array_assign.hpp"

#include <cassert>

bool test_assign_to_array1D()
{
  int a[2], b[2];
  ltl::assign_to{a} = {1,2};
  ltl::assign_to{b} = a;
  assert( a[0] == 1 && b[0] == 1
       && a[1] == 2 && b[1] == 2);
  ltl::assign_to{b} = {};
  assert( b[0] == 0 && b[1] == 0 );

  return true;
}

bool test_assign_to_array2D()
{
  int a[3][2], b[3][2];
  ltl::assign_to{a} = {{1,2},{3,4},{5,6}};
  ltl::assign_to{b} = a;
  assert( a[0][0] == 1 && b[0][0] == 1
       && a[1][0] == 3 && b[1][0] == 3);
  ltl::assign_to{b} = {};
  assert( b[0][0] == 0 && b[1][0] == 0 );
  ltl::assign_to{b[0]} = a[2];
  assert( b[0][0] == 5 && b[0][1] == 6 );

  return true;
}

bool test_assign_scalar()
{
  int a, b;
  ltl::assign(a) = 1;
  ltl::assign(b) = {1};
  assert( a == 1 && b == 1 );
  ltl::assign(a) = 0;
  ltl::assign(b) = {};
  assert( a == 0 && b == 0 );

  return true;
}

bool test_assign_array1D()
{
  int a[2], b[2];
  ltl::assign(a) = {1,2};
  ltl::assign(b) = a;
  assert( a[0] == 1 && b[0] == 1
       && a[1] == 2 && b[1] == 2);
  ltl::assign(b) = {};
  assert( b[0] == 0 && b[1] == 0 );

  return true;
}

bool test_assign_array2D()
{
  int a[3][2], b[3][2];
  ltl::assign(a) = {{1,2},{3,4},{5,6}};
  ltl::assign(b) = a;
  assert( a[0][0] == 1 && b[0][0] == 1
       && a[1][0] == 3 && b[1][0] == 3);
  ltl::assign(b) = {};
  assert( b[0][0] == 0 && b[1][0] == 0 );
  ltl::assign(b[0]) = a[2];
  assert( b[0][0] == 5 && b[0][1] == 6 );

  return true;
}

bool test_assign_elements()
{
  int a[3][2], b[3][2];
  ltl::assign_elements(ltl::flat_cast(a), 0,1,2,3,4,5);
  assert(a[0][0] == 0 && a[2][1] == 5);
  ltl::assign_elements(b, a[2],a[1],a[0]);
  assert(b[2][0] == 0 && b[0][1] == 5);
  ltl::assign_elements(a[0], b[0][1],b[0][0]);
  assert(a[0][0] == 5);

  struct move_only {
    move_only()=default;
    move_only(move_only&&)=default;
    move_only& operator=(move_only&&)=default;
  };
  static_assert(   ltl::is_move_assignable_v<move_only> );
  static_assert( ! ltl::is_copy_assignable_v<move_only> );

  move_only moa[2]{};
  ltl::assign_elements(moa, (move_only&&)moa[1], (move_only&&)moa[0]);

  return true;
}

int main()
{
  test_assign_to_array1D();
  test_assign_to_array2D();
  test_assign_scalar();
  test_assign_array1D();
  test_assign_array2D();
  test_assign_elements();

  wrap<int> wi{2};
  auto& [wiv] = wi;
  assert( wiv == 2 );
  ltl::assign(wi) = {};
  assert( wiv == 0 );
  ltl::assign(wi) = {2};
  assert( wiv == 2 );
}
