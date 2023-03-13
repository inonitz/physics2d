#pragma once
#include "vec.hpp"
#include "matrix.hpp"




using Vec3 = math::vec3f;
using Vec4 = math::vec4f;
using Mat4 = math::mat4f;
using Pressure = f32;


struct GridCell {
	Vec3 	 pos; /* needs to have integer indices :/ */
	Vec3 	 vel;
	Vec3 	 tmp;
	Pressure p;
	u8 layer;
	u8 type; /* Air, Fluid, Solid. */
};


struct Fluid
{
	f32 dx;
	f32 u_max;
	f32 k_cfl;
	f32 dt;	
};


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


// template<u16 __N> void SSOR_Preconditioner(Matrixf<__N> A, Matrixf<__N>& M, Matrixf<__N>& N)
// {
// 	Matrixf<__N> L{0.0f};
// 	Vectorf<__N> invD;

// 	for(size_t i = 0; i < __N; ++i) {
// 		invD = (1.0f / )
// 	}
// }


template<u16 N> void PCG_SolveZ(Matrixf<N> M, Vectorf<N>& z, Vectorf<N> r)
{

}


template<u16 N> void PreconditionedConjugateGradient(Matrixf<N> A, Matrixf<N> M, Vectorf<N>& initialX, Vectorf<N> b, f32 tolerance = 1e-6f, u16 max_iter = 100)
{
	Vectorf<N> residue = b, z, p, w, tmp;
	f32 alpha, beta;
	
	if(initialX.magnitude() > 1.0f) { /* This is expensive, so just incase initialX is NOT the 0 vector. */
		b -= A * initialX;
	}


	PCG_SolveZ(M, z, residue); /* z_0 */
	p = z; /* p1 */
	w = A * p; 
	tmp = Vectorf<N>::dot(residue, z); /* r0_T * z0 */
	alpha = tmp * (1.0f / Vectorf<N>::dot(p, w)); /* above / (p1_T * W) */
	initialX += alpha * p; /* x1 = x0 + a1*p1 */
	residue  -= alpha * w; /* r1 = r0 - a1*w */


	for(size_t i = 0; residue.magnitude() > tolerance && i < max_iter; ++i)
	{
		PCG_SolveZ(M, z, residue); /* solve for z_k */
		beta = Vectorf<N>::dot(residue, z) * (1.0f / tmp); /* beta_k */
		tmp  = Vectorf<N>::dot(residue, z);

		p = z + beta * p; /* p_z+1 */
		w = A * p;
		alpha = tmp * (1.0f / Vectorf<N>::dot(p, w)); /* a_k+1 */
		initialX -= alpha * p; /* x_k+1 */
		residue  -= alpha * w; /* r_k+1 */
	}

	return;
}