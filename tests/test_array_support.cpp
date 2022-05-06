#include "test_array_support.hpp"

//#include "ALLOW_ZERO_SIZE_ARRAY.hpp"
//ALLOW_ZERO_SIZE_ARRAY
//(
//static_assert( std::is_same_v<std::decay_t<int[0]>,int[0]> );
//)
//#include "ALLOW_ZERO_SIZE_ARRAY.hpp"

int main()
{
    int mint2[2] {1,2};
    int mint23[2][3] {{1,2,3},{4,5,6}};

// flat_index has non is_constant_evaluated() code

    bool flat_index_test =
    ltl::flat_index(int23{{1,2,3},{4,5,6}}) == 1
 && ltl::flat_index(int23{{1,2,3},{4,5,6}},3) == 4
 && ltl::flat_index(mint23) == 1
 && ltl::flat_index(mint23,4) == 5
 && ltl::flat_index(int2{1,2},1) == 2
 && ltl::flat_index(mint2,1) == 2;

    int mint4213[4][2][1][3]
    {  { {{0,1,2}},{{3,4,5}} },
       { {{6,7,8}},{{9,0,1}} },
       { {{2,3,4}},{{5,6,7}} },
       { {{8,9,0}},{{1,2,3}} } };

    flat_index_test = flat_index_test &&
    ltl::flat_index(mint4213,0) == 0
 && ltl::flat_index(mint4213,1) == 1
 && ltl::flat_index(mint4213,2) == 2
 && ltl::flat_index(mint4213,3) == 3
 && ltl::flat_index(mint4213,5) == 5
 && ltl::flat_index(mint4213,6) == 6
 && ltl::flat_index(mint4213,8) == 8
 && ltl::flat_index(mint4213,23) == 3;

 return ! flat_index_test;
}
#if 0
#endif
