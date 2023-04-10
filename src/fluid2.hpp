#pragma once
#include "vec.hpp"
#include "matrix.hpp"
#include <cmath>




constexpr f32 DivByZeroEpsilon = 1e-20;


#define DIVISOR_NOT_ZERO(val) (val + DivByZeroEpsilon)
#define INVERSE(val) (1.0f / DIVISOR_NOT_ZERO(val))


using Vec3 = math::vec3f;
using Vec4 = math::vec4f;
using Mat4 = math::mat4f;
using GridPosition = math::vec3u;
using Pressure = f32;
using Velocity = f32;


#define LERP(val_x, val_y, __t) (val_y * __t + (1.0f - __t) * val_x) \


#define BILERP(val_x, val_y, val_z, val_w, __t0, __t1) \
	LERP( \
		LERP(val_x, val_y, __t0), \
		LERP(val_z, val_w, __t0), \
		__t1 \
	) \


#define TRILERP(coef1to4, coef5to8, vec3) \
	LERP( \
		BILERP(coef1to4.m0, coef1to4.m1, coef1to4.m2, coef1to4.m3, vec.x, vec3.y), \
		BILERP(coef5to8.m0, coef5to8.m1, coef5to8.m2, coef5to8.m3, vec.x, vec3.y), \
		vec3.z \
	); \


inline f32 BiLerp(math::vec4f const& coef, math::vec2f const& vec)
{
	// math::vec2f tmp = {
	// 	LERP(coef.m0, coef.m1, vec.x),
	// 	LERP(coef.m2, coef.m3, vec.x)
	// };
	// return LERP(tmp.x, tmp.y, vec.y);
	return BILERP(coef.m0, coef.m1, coef.m2, coef.m3, vec.x, vec.y);
}



inline f32 TriLerp(math::vec4f const& coef0, math::vec4f const& coef1, math::vec3f const& vec)
{
	return LERP(
		BILERP(coef0.m0, coef0.m1, coef0.m2, coef0.m3, vec.x, vec.y),
		BILERP(coef1.m0, coef1.m1, coef1.m2, coef1.m3, vec.x, vec.y),
		vec.z
	);
}



/* Solve for Linear System Ax = b, where A is a Unit-Lower-Triangular Matrix. */
template<u16 __N> void BackwardSolver(Matrixf<__N> A, Vectorf<__N>& x, Vectorf<__N> b)
{
	f32 sum = 0.0f;


	x[0] = b[0];
	for(size_t i = 1; i < __N; ++i) 
	{
		sum = 0.0f;
		for(size_t j = 0; j < i; ++j) { /* sum[ A(row i from 0:i) * x(from 0:i) ]*/
			sum += A(i, j) * x[j];
		}
		x[i] = b[i] - sum;
	}
}


template<u16 __N> void ForwardSolver(Matrixf<__N> A, Vectorf<__N>& x, Vectorf<__N> b)
{
	f32 sum = 0.0f;
	
	x[__N - 1] = b[__N - 1] * ( 1.0f/A(__N - 1, __N - 1) ); /* x[n] = b[n] - A[n, n] */
	for(size_t i = __N - 2; i >= 0; --i) 
	{
		sum = 0.0f;
		for(size_t j = i; j < __N; ++j) { /* sum[ A(row i from i:n) * x(from i:n) ]*/
			sum += A(i, j) * x[j];
		}

		x[i] = b[i] - sum;
		x[i] *= (1.0f / A(i, i));
	}
	return;
}



/* 
	Matrix 'L' MUST be init to 0!
	A will be modified, make sure it is a copy.
	(
		Unfortunately I didn't spend enough time to optimize the algorithm,
		besides extracting the diagonal first. 
	)
*/
// template<u16 __N> void IncompleteCholeskyPreconditioner(Matrixf<__N> A, Matrixf<__N>& L)
// {
// 	Vectorf<__N> diagRoots{0.0f};
// 	f32 tmp{0.0f};
	
// 	for(u16 i = 0; i < __N; ++i) {
// 		diagRoots[i] = INVERSE( std::sqrt(A(i, i)) ); 
// 	}


// 	for(u16 k = 0; k < __N - 1; ++k)
// 	{
// 		for(u16 i = k + 1; i < __N; ++i) {
// 			L(k, i) = A(k, i) * diagRoots[k];
// 		}
// 		for(u16 i = k + 1; i < __N; ++i) 
// 		{	
// 			/* Blocking Can Improve the performance of this significantly for better cache usage. */
// 			for(u16 j = i; j < __N; ++j) {
// 				tmp = L(k, i) * L(k, j); /* element-wise-mul between the upper row in L to the upper column in L */
// 				tmp *= boolean( A(i, j) != 0.0f );
// 				A(i, j) -= tmp;
// 			}
// 		}
// 	}
// 	return;
// }


/* 
	Use SparseMatrix Struct for this, 
	this is inefficient as hell without a bitmap or something (or Blocking for that matter lol)
	Source (Only Pseudocode Correct): https://en.wikipedia.org/wiki/Incomplete_Cholesky_factorization

	NOTE: This is The Triangular-Lower Part L. The True preconditioner is (L * L^T)
*/
template<u16 __N> void IncompleteCholeskyPreconditioner(Matrixf<__N>& A)
{
	Vectorf<__N> diagRoots{0.0f};
	// f32 tmp{0.0f};
	
	for(u16 i = 0; i < __N; ++i) {
		diagRoots[i] = INVERSE( std::sqrt(A(i, i)) ); 
	}


	for(size_t k = 0; k < __N; ++k)
	{
		for(size_t i = k + 1; i < __N; ++i) {
			if(A(i, k) != 0.0f) { A(i, k) *= diagRoots[k]; }
		}
		
		for(size_t j = k + 1; j < __N; ++j) {
			for(size_t i = j; i < __N; ++i) {
				A(i, j) -= A(i, k)*A(j, k);
			}
		}
	}
	for(size_t i = 0; i < __N; ++i) { 
		std::fill(&A.data()[i+1 + i*__N], &A.data()[__N - 1 + i*__N], 0.0f);
	}


	return;
}




template<u16 N> bool PreconditionedConjugateGradient(Matrixf<N> A, Matrixf<N> Minv, Vectorf<N>& initialX, Vectorf<N> b, f32 tolerance = 1e-6f, u16 max_iter = 100)
{
	Vectorf<N> residue = b, z, p, Ap, tmp;
	f32 alpha, beta;


	if(initialX.mag() > 1.0f) { /* This is expensive, so just incase initialX is NOT the 0 vector. */
		residue -= A * initialX;
	}


	z = Minv * residue; /* z_0 = M^-1 * r0 */
	p = z; /* p1 */
	Ap = A * p; /* Save computation cost */


	alpha = INVERSE(dot(p, Ap));       /* (p1_T * Ap)^-1  */
	tmp   = dot(residue, z);      /* r0_T * z0 	  */
	alpha *= tmp;
	initialX += alpha * p; 				/* x1 = x0 + a1*p1 */
	residue  -= alpha * Ap; 			/* r1 = r0 - a1*Ap0*/
	--max_iter;


	do {
		z = Minv * residue; 			   /* z_k = M^-1 * r_k */
		beta = dot(residue, z) * tmp; /* beta_k =  */
		p = z + beta * p;				   /* p_k+1  */
		Ap = A * p; 					   /* Ap_k+1 */
		

		tmp   = dot(residue, z);
		alpha = INVERSE(dot(p, Ap));
		alpha *= tmp; 			           /* a_k+1 */
		initialX += alpha * p;             /* x_k+1 */
		residue  -= alpha * Ap;            /* r_k+1 */

		--max_iter;
	} while(max_iter && residue.mag() > tolerance);


	return max_iter == 0; /* Success if not reached 0. */
}