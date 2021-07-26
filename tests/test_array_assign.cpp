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
  ltl::assign(a,1);
  ltl::assign(b,{1});
  assert( a == 1 && b == 1 );
  ltl::assign(b,{});
  assert( b == 0 );

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

  ltl::assign(a,{3,4});
  ltl::assign(b,a);
  assert( a[0] == 3 && b[0] == 3
       && a[1] == 4 && b[1] == 4);
  ltl::assign(b,{});
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
  ltl::assign(b,{});
  assert( b[0][0] == 0 && b[1][0] == 0 );
  ltl::assign(b[0],a[2]);
  assert( b[0][0] == 5 && b[0][1] == 6 );

  return true;
}

int main()
{
  test_assign_to_array1D();
  test_assign_to_array2D();
  test_assign_scalar();
  test_assign_array1D();
  test_assign_array2D();

  wrap<int> wi{2};
  auto& [wiv] = wi;
  assert( wiv == 2 );
  ltl::assign(wi) = {};
  assert( wiv == 0 );
  ltl::assign(wi) = {2};
  assert( wiv == 2 );
}
