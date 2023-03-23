#include "test_matrices.hpp"




template<u16 __N> void Solve(Matrixf<__N> A, Vectorf<__N> x, Vectorf<__N> b)
{
    Matrixf<__N> tmp = A, tmp2;
    Matrixf<__N> precon0{0.0f};
}




int test_matrices()
{
    u8 status = 0;


    Matrixf<256> matA{0.0f}, matB;


    matA = Matrixf<256>{1.0f};

    return status;
}