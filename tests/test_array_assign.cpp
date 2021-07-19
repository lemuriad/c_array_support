#include "test_array_assign.hpp"

#include <cassert>

int main() {
  wrap<int> wi{2};
  auto& [wiv] = wi;
  assert( wiv == 2 );
  ltl::assign{}(wi);
  assert( wiv == 0 );
  ltl::assign{}(wi,{2});
  assert( wiv == 2 );
}
