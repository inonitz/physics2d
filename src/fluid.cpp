#include "fluid.hpp"
#include <string.h>




template<
    typename T, 
    bool     isMatrix = false, 
    bool     transposedForOptimizedMul = false
> struct DynamicVector {
public:
    T*     m_data;
    size_t m_size;
    size_t m_rowSize;


    __force_inline bool matrix() const { return isMatrix; }

private:
    __force_inline void allocate_data() { m_data = __scast(T*, _mm_malloc(  m_size * sizeof(T), math::round2(sizeof(T))  )); }
    using HVector = DynamicVector<T, false>;
    using HMatrix = DynamicVector<T, true>;

public:
    DynamicVector() : m_data{nullptr}, m_size{0}, m_rowSize{0}{}


    void create(u32 generic_length)
    {
        m_size    = generic_length;
        m_rowSize = isMatrix * generic_length + (!isMatrix); /* if matrix set rowSize to N, else keep as 1. */
        m_size   *= isMatrix * generic_length + (!isMatrix); /* if matrix set to NxN, else keep as original */

        allocate_data();
        return;
    }


    void destroy()
    {
        if(boolean(m_data == nullptr)) return;
        _mm_free(m_data);
        m_size = 0;
        m_rowSize = 0;
        return;
    }


    void copy(DynamicVector& in)
    {
        memcpy(m_data, in.m_data, in.bytes());
        m_size = in.m_size;
        m_rowSize = in.m_rowSize;
        return;
    }


    void set(math::const_ref<T> initial = __scast(T, 0))
    {
        size_t i = 0;
        for(; i < (m_size / 4); ++i) {
            m_data[4*i + 0] = initial;
            m_data[4*i + 1] = initial;
            m_data[4*i + 2] = initial;
            m_data[4*i + 3] = initial;
        }
        for(i *= 4; i < m_size; ++i) { m_data[i] = initial; }
    }

    
    void set(DynamicVector& in)
    {
        ifcrashdbg(in.size() == m_size);
        copy(in);
        return;
    }


    T&       operator[](size_t idx        )       { ifcrashdbg(m_data == nullptr); return m_data[idx]; }
    T const& operator[](size_t idx        ) const { ifcrashdbg(m_data == nullptr); return m_data[idx]; }
    T&       operator()(size_t i, size_t j)       { ifcrashdbg(m_data == nullptr); return m_data[j + i * m_rowSize]; }
    T const& operator()(size_t i, size_t j) const { ifcrashdbg(m_data == nullptr); return m_data[j + i * m_rowSize]; }


    size_t row() const { return m_rowSize; }


    static void dot(__unused HMatrix& out, __unused HMatrix& A, __unused HMatrix& B)
    {
        return;
        // for(size_t i = 0; i < A.row(); ++i)
        // {
        //     for(size_t i = 0; i < A.row(); ++i)
        // }
    }


    static void dot(__unused HVector& out, __unused HMatrix& A, __unused HVector& B)
    {
        return;
    }


    static T dot(HVector& A, HVector& B)
    {
        ifcrashdbg(A.size() != B.size());
        
        math::Vector<T, 4> tmp; T aggregate{0x00};
        size_t i = 0;
        for(; i < A.size() / 4; ++i) {
            tmp[0] = A.m_data[4 * i    ] * B.m_data[4 * i    ];
            tmp[1] = A.m_data[4 * i + 1] * B.m_data[4 * i + 1];
            tmp[2] = A.m_data[4 * i + 2] * B.m_data[4 * i + 2];
            tmp[3] = A.m_data[4 * i + 3] * B.m_data[4 * i + 3];
            tmp[i    ] = tmp[i	  ] + tmp[i + 1];
            tmp[i + 1] = tmp[i + 2] + tmp[i + 3];
            tmp[i] 	   = tmp[i]     + tmp[i + 1];
            
            aggregate += tmp[i];
        }
        for(i *= 4; i < A.size(); ++i) { aggregate += A.m_data[i] * B.m_data[i]; }
        return aggregate;
    }



#define DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION(op_symbol, opname, arg2ScalarBool, n0, n1, n2, type0, type1, type2, arr_access_member) \
    static void opname( \
        type0 n0, \
        type0 n1, \
        type2 n2 \
    ) { \
        /* if [out, A, B] are ALL matrices/vectors, check for size compatibillity if we don't want buffer overflows. */ \
        if constexpr(arg2ScalarBool) { \
            ifcrashdbg((  \
                ( \
                    (n0.matrix() && n1.matrix()) || (!n0.matrix() && !n1.matrix()) \
                ) && (n0.size() != n1.size()) \
            )); \
        } else { \
            ifcrashdbg((  \
                ( \
                    (n0.matrix() && n1.matrix() && n2.matrix()) || (!n0.matrix() && !n1.matrix() && !n2.matrix()) \
                ) && (n0.size() != n1.size() && n1.size() != n2.size()) \
            )); \
        } \
        size_t i = 0; \
        for(; i < (n0.size() / 4); ++i) { \
            if constexpr(arg2ScalarBool) { \
                n0.arr_access_member[4 * i + 0] = n1.arr_access_member[4 * i + 0] op_symbol n2; \
                n0.arr_access_member[4 * i + 1] = n1.arr_access_member[4 * i + 1] op_symbol n2; \
                n0.arr_access_member[4 * i + 2] = n1.arr_access_member[4 * i + 2] op_symbol n2; \
                n0.arr_access_member[4 * i + 3] = n1.arr_access_member[4 * i + 3] op_symbol n2; \
            } else { \
                n0.arr_access_member[4 * i + 0] = n1.arr_access_member[4 * i + 0] op_symbol n2.arr_access_member[4 * i + 0]; \
                n0.arr_access_member[4 * i + 1] = n1.arr_access_member[4 * i + 1] op_symbol n2.arr_access_member[4 * i + 1]; \
                n0.arr_access_member[4 * i + 2] = n1.arr_access_member[4 * i + 2] op_symbol n2.arr_access_member[4 * i + 2]; \
                n0.arr_access_member[4 * i + 3] = n1.arr_access_member[4 * i + 3] op_symbol n2.arr_access_member[4 * i + 3]; \
            } \
        } \
        for(i *= 4; i < n0.size(); ++i) { \
            if constexpr(arg2ScalarBool) { \
                n0.arr_access_member[i] = n1.arr_access_member[i] op_symbol n2; \
            } else { \
                n0.arr_access_member[i] = n1.arr_access_member[i] op_symbol n2.arr_access_member[i]; \
            } \
        } \
        return; \
    }


#define GENERATE_VECTOR_OPERATOR_FUNCTIONS(op_symbol, name) \
DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION(op_symbol, name, false, out, A, B, HVector const&, HVector const&, HVector const&, m_data); \
DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION(op_symbol, name, true,  out, A, B, HVector const&, HVector const&, T               m_data); \

DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION(+, addMatrix, true,  out, A, B, HMatrix&, HMatrix&, HMatrix&, m_data);
DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION(+, addVector, false, out, A, B, HVector&, HVector&, HVector&, m_data);
DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION(+, addScalar, false, out, A, B, HVector&, HVector&, T,        m_data);
DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION(-, subMatrix, true,  out, A, B, HMatrix&, HMatrix&, HMatrix&, m_data);
DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION(-, subVector, false, out, A, B, HVector&, HVector&, HVector&, m_data);
DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION(-, subScalar, false, out, A, B, HVector&, HVector&, T,        m_data);
DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION(*, mulMatrix, true,  out, A, B, HMatrix&, HMatrix&, HMatrix&, m_data);
DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION(*, mulVector, false, out, A, B, HVector&, HVector&, HVector&, m_data);
DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION(*, mulScalar, false, out, A, B, HVector&, HVector&, T,        m_data);
#undef DEFINE_MATRIX_GENERIC_OPERATION_FUNCTION
#define DEFINE_MIN_MAX_OPERATION(op_symbol, name) \
    T name() const { \
        math::ref<T> out = m_data[0]; \
        for(size_t i = 1; i < m_size; ++i) { out = (m_data[i] op_symbol out) ? m_data[i] : out; } \
        return out; \
    } \

DEFINE_MIN_MAX_OPERATION(>, max);
DEFINE_MIN_MAX_OPERATION(<, min);
#undef DEFINE_MIN_MAX_OPERATION


    __force_inline static T magnitude (HVector& in) { return std::sqrt(dot(in, in)); }
    __force_inline static T magnitude2(HVector& in) { return dot(in, in);            }
    size_t         size() const { return m_size;             }
    size_t        bytes() const { return m_size * sizeof(T); }
    T*            begin() const { return m_data;             }
    T*              end() const { return &m_data[m_size];    }
    T*             data() const { return m_data;             }
    T const* const_data() const { return m_data;             }
};




template<size_t size> using StaticVectorf = math::Vector<f32, size>;
template<size_t size> using SVectorf      = StaticVectorf<size>;
template<typename T>  using HVector       = DynamicVector<T, false>;
template<typename T>  using HMatrix       = DynamicVector<T, true>;
using HVectorf = HVector<f32>; /* Heap Vector-float */
using HMatrixf = HMatrix<f32>;




math::vec2f Fluid2D::velocity(VectorField* u, u32 i, u32 j) const
{
    u32 xIdx = i + (i / N);
    u32 yIdx = j + (j / N);
    return {
        0.5f * (u->x[xIdx] + u->x[xIdx + 1]),
        0.5f * (u->y[yIdx] + u->y[yIdx + 1]),
    };
}


math::vec2f Fluid2D::position(u32 i, u32 j) const
{
    return {
        __scast(f32,          i) * dx, /* i mapped from [0, N-1] => [0,      (N-1)dx] */
        __scast(f32, N - 1u - j) * dx  /* j mapped from [0, N-1] => [(N-1)dx,      0] */
    };
}


__force_inline math::vec2u Fluid2D::indices(math::vec2f const& pos)
{
    /* 
        Inverse of position(i, j); since we're working with floats and pos might not be 
        on the grid, we need to take the whole part of each component, and split it into a range:
        given pos = (a.x, b.y), we return (a, b, a+1, b+1);
    */
    math::vec2f idx = pos * ( recpdx * __scast(f32, N) );
    math::vec2u out = {
        __scast(u32, idx.x),
        __scast(u32, idx.y),
    };
    /* Not returning out + 1 since it can be calculated outside this function. */
    return {
        out.i, 
        out.j,
    };

}


void applyPrecondition(__unused HVectorf& out, __unused HVectorf& v)
{
    return;
}


void applyEquationMatrix(__unused HVectorf& out, __unused HMatrixf& A, __unused HVectorf& q)
{
    return;
}


// void preconditioned_conjugate_gradient(
//     f32 density, 
//     HVectorf& tmp, 
//     HVectorf& s, 
//     HVectorf& z, 
//     HVectorf& r, 
//     HMatrixf& A, 
//     HVectorf& p, 
//     HVectorf& d
// ) {
//     u32 max_iter = 100, i = 0;
//     f32 r_max    = 1.0f, threshold = 1e-5, sigma, alpha;
    

//     p.set(0.0f);                /* p = 0         */
//     r.set(d);                       /* r = d         */
//     applyPrecondition(z, r);
//     s.set(z);                       /* s = z         */
//     sigma = HVectorf::dot(z, r); /* sigma = z * r */
//     while(r_max > threshold && i < max_iter) /* loop until threshold reached / we've iterated too much. */
//     {
//         applyEquationMatrix(z, A, s);                 /* */
//         alpha = density * ( 1.0f / HVectorf::dot(z, s) ); /* alpha = density / (z * s) */

//         HVectorf::mulScalar(tmp, s, alpha);
//         HVectorf::addVector(p, p, tmp);
//         HVectorf::subVector(r, r, tmp);


//         r_max = r.max();
//         applyPrecondition(z, r);
//         // sigma = 
//         /* 
//             TODO continue writing this, but for the love of god simplify this, this is horrible code.
//             Keep reading from the article and try to understand the algorithm completely before you continue.
//             (https://www.cs.ubc.ca/~rbridson/fluidsimulation/fluids_notes.pdf)
//         */

        
//         /* ri = Ai * pi - di */



//         HMatrixf::dot(r, A, p);
//         HMatrixf::subVector(r, d, r);
//         r_max = HVectorf::magnitude(r);


//         ++i;
//     }
// }

// void PCG()
// {

// }


void Fluid2D::advect(VectorField* u, f32 dt, VectorField* q)
{
    math::vec2f xp{}, xj{}, alpha{};
    math::vec2u jPos;
    for(size_t i = 0; i < N; ++i)
    {
        for(size_t j = 0; j < N; ++j)
        {
            /* 
                Full Equation:
                    1. xP = x[i] - dt * u[i]
                    2. [ x[j], x[j+1] ] are grid positions where xP lies in-between them
                    3. a = (xp - xj) / dx
                    ==> 4. q[i]_n+1 = (1 - a) * q[j]_n + a * q[j+1]_n,

            */
            xp   = position(i, j) - dt * velocity(u, i, j);
            jPos = indices(xp);
            xj   = position(jPos.i, jPos.j);
            alpha = (1.0f / dx) * (xp - xj);
            
            q->x[i] = (1.0f - alpha.x) * q->x[jPos.i] + alpha.x * q->x[jPos.i + 1];
            q->y[j] = (1.0f - alpha.y) * q->y[jPos.j] + alpha.y * q->y[jPos.j + 1];
        }
    }
    return;
}


// void Fluid2D::project(f32 dt, VectorField* u)
// {

// }




void Fluid2D::stepFrame(f32 dt)
{
    /* 
        Since in the full simulation we need to use the Residual of the Linear System Of Equations, 
        we'll just a constant max_iter for now. 
    */
    u32 max_iter = 20;
    math::vec2f u_max = {};


    for(u32 n = 0; n < max_iter; ++n)
    {
        advect(&v, dt, &v0);

    }
}
