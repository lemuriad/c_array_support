#include "test_array_assign.hpp"

#include <cassert>

int main() {
  wrap<int> wi{2};
  ltl::assign{}(wi);
  auto [wiv] = wi;
  assert( wiv == 0 );
}
