#pragma once
#include "../base.hpp"
#include "../manvec.hpp"
#include <math.h>
#include <array>


#define NAMESPACE_MATH_BEGIN namespace math {
#define NAMESPACE_MATH_END   }




NAMESPACE_MATH_BEGIN




template<typename T> using ref = typename std::conditional<sizeof(T) <= 8, T, T&>::type;


template<typename T> constexpr T round2(T v) {
	static_assert(std::is_integral<T>::value, "Value must be an Integral Type! (Value v belongs to group N [0 -> +inf]. ");
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	++v;
	return v;
}




template<typename T, size_t length> class Vector
{
private:
    T __data[round2<size_t>(length)];
    
public:
	void zero() { memset(__data, 0x00, bytes()); return; }


	Vector() { zero(); }
	Vector(ref<T> defaultVal) 
	{
		for(size_t i = 0; i < length; i <<= 1)  {
			__data[i    ] = defaultVal;
			__data[i + 1] = defaultVal;
		}
		return;
	}


	

	T& 		 operator[](uint8_t idx) 	   { ifcrashdbg((size_t)idx >= length); return __data[idx]; }
	const T& operator[](uint8_t idx) const { ifcrashdbg((size_t)idx >= length); return __data[idx]; }
	

	      T* begin()       { return &__data[0];      }
	      T* end()         { return &__data[length]; }
	const T* begin() const { return &__data[0];      }
	const T* end()   const { return &__data[length]; }
	constexpr size_t bytes()  const { return sizeof(T) * length;  }
	constexpr size_t len()    const { return length;		      }
};


static Vector<float, 2 > temporaryBufferVec2f{};
static Vector<float, 3 > temporaryBufferVec3f{};
static Vector<float, 4 > temporaryBufferVec4f{};
static Vector<float, 16> temporaryBufferMat4f{};
static Vector<u32,   2 > temporaryBufferVec2u{};
static Vector<u32,   3 > temporaryBufferVec3u{};
static Vector<u32,   4 > temporaryBufferVec4u{};
static Vector<i32,   2 > temporaryBufferVec2i{};
static Vector<i32,   3 > temporaryBufferVec3i{};
static Vector<i32,   4 > temporaryBufferVec4i{};


#define SET_MULTIPLE_VALUES 4llu


#define INTRMD_FUNC_GENERATOR(op_symbol, name, arg1a0, arg1a1, arg1a2, arg1a3, ...) \
template<typename T, size_t len> void name( \
	Vector<T, len> const& a, \
    __VA_ARGS__            , \
	Vector<T, len>& 	  dst \
) { \
	Vector<T, SET_MULTIPLE_VALUES> tmp; \
	size_t i  = 0; \
	\
	for(; i < len / SET_MULTIPLE_VALUES; ++i) { \
		tmp[i    ] = a[i    ] op_symbol arg1a0; \
		tmp[i + 1] = a[i + 1] op_symbol arg1a1; \
		tmp[i + 2] = a[i + 2] op_symbol arg1a2; \
		tmp[i + 3] = a[i + 3] op_symbol arg1a3; \
		memcpy(&dst.begin()[i], tmp.begin(), tmp.bytes()); \
	} \
	for(i *= SET_MULTIPLE_VALUES; i < len; ++i) { dst[i] = a[i] op_symbol arg1a0; } \
	return; \
} \


#define GENERATE_VECTOR_OPERATOR_FUNCTIONS(op_symbol, name) \
INTRMD_FUNC_GENERATOR(op_symbol, name, b[i], b[i+1], b[i+2], b[i+3], Vector<T, len> const& b) \
INTRMD_FUNC_GENERATOR(op_symbol, name, b, b, b, b, T b) \



GENERATE_VECTOR_OPERATOR_FUNCTIONS(+, add)
GENERATE_VECTOR_OPERATOR_FUNCTIONS(-, sub)
GENERATE_VECTOR_OPERATOR_FUNCTIONS(*, mul)
GENERATE_VECTOR_OPERATOR_FUNCTIONS(/, div)


template<typename T, size_t len> T dot_prod(
	Vector<T, len> const& a, 
	Vector<T, len> const& b
) {
	Vector<T, SET_MULTIPLE_VALUES> tmp;
	T aggregate{0x00};
	size_t i  = 0;

	for(; i < len / SET_MULTIPLE_VALUES; ++i) {
		tmp[i    ] = a[i    ] * b[i    ];
		tmp[i + 1] = a[i + 1] * b[i + 1];
		tmp[i + 2] = a[i + 2] * b[i + 2];
		tmp[i + 3] = a[i + 3] * b[i + 3];
		tmp[i    ] = tmp[i	  ] + tmp[i + 1];
		tmp[i + 1] = tmp[i + 1] + tmp[i + 2];
		tmp[i] 	   = tmp[i] 	+ tmp[i + 1];
		
		aggregate += tmp[i];
	}
	for(i *= SET_MULTIPLE_VALUES; i < len; ++i) { aggregate += a[i] * b[i]; }
	return aggregate;
}


template<typename T, size_t len> void norm(Vector<T, len>& a) {
	T tmp = dot_prod(a, a); tmp = ((T)1) / tmp;
	mul<T, len>(a, tmp, a);
	return;
}


template<typename T, size_t len> T length_sqr(Vector<T, len> const& a) { return dot_prod(a, a);       }
template<typename T, size_t len> T vec_length(Vector<T, len> const& a) { return sqrt(dot_prod(a, a)); }



template<typename T> void cross_prod(
	Vector<T, 3> const& a, 
	Vector<T, 3> const& b, 
	Vector<T, 3>&       dst
) {
	dst[0] = a[2] * b[3] - b[2] * a[3];
	dst[1] = a[3] * b[1] - b[3] * a[1];
	dst[2] = a[1] * b[2] - b[1] * a[2];
	return;
}




#undef SET_MULTIPLE_VALUES


#define DEFINE_VECTOR_STRUCTURE_ARGS(len, dtype, aptn, declare_union_structs, declare_n_args_ctor, ...) \
struct vec##aptn \
{ \
    union { \
        Vector<dtype, len> mem; \
        \
        \
        declare_union_structs; \
    }; \
\
\
    vec##aptn() : mem() {} \
    vec##aptn(dtype value) : mem(value) {} \
    declare_n_args_ctor \
    vec##aptn(const dtype* validAddr) { \
        memcpy(mem.begin(), validAddr, mem.bytes()); \
        return; \
    } \
    vec##aptn(std::array<dtype, len> const& arr) : vec##aptn(arr.begin()) {} \
    vec##aptn(Vector<dtype, len>     const& vec) : mem(vec)               {} \
	vec##aptn(vec##aptn 			 const& cpy) : mem(cpy.mem) 		  {} \
    vec##aptn& operator=(const vec##aptn& cpy) { \
        memcpy(begin(), cpy.begin(), bytes()); \
        return *this; \
    } \
\
\
    __force_inline 		 Vector<dtype, len>& operator+(vec##aptn const& b) 		 { add(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
    __force_inline 		 Vector<dtype, len>& operator-(vec##aptn const& b) 		 { sub(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
    __force_inline 		 Vector<dtype, len>& operator*(vec##aptn const& b) 		 { mul(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
    __force_inline 		 Vector<dtype, len>& operator/(vec##aptn const& b) 		 { div(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline 		 Vector<dtype, len>& operator*(dtype            a) 		 { mul(mem, a,     temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
    __force_inline 		 Vector<dtype, len>& operator/(dtype            a) 		 { div(mem, a,     temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
    __force_inline const Vector<dtype, len>& operator+(vec##aptn const& b) const { add(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline const Vector<dtype, len>& operator-(vec##aptn const& b) const { sub(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline const Vector<dtype, len>& operator*(vec##aptn const& b) const { mul(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline 		 Vector<dtype, len>& operator/(vec##aptn const& b) const { div(mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline const Vector<dtype, len>& operator*(dtype     const  a) const { mul(mem, a,     temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline const Vector<dtype, len>& operator/(dtype     const  a) const { div(mem, a,     temporaryBufferVec##aptn); return temporaryBufferVec##aptn;  } \
	__force_inline vec##aptn& operator+=(vec##aptn const& b) { add(mem, b.mem, mem); return *this;  } \
    __force_inline vec##aptn& operator-=(vec##aptn const& b) { sub(mem, b.mem, mem); return *this;  } \
    __force_inline vec##aptn& operator*=(vec##aptn const& b) { mul(mem, b.mem, mem); return *this;  } \
    __force_inline vec##aptn& operator*=(dtype            a) { mul(mem, a,     mem); return *this;  } \
    __force_inline vec##aptn& operator/=(dtype            b) { div(mem, b,     mem); return *this;  } \
\
	__force_inline dtype               length()     const     		 { temporaryBufferVec##aptn = mem; 								   return vec_length(temporaryBufferVec##aptn); } \
    __force_inline dtype               lensqr()     const     		 { temporaryBufferVec##aptn = mem; 								   return length_sqr(temporaryBufferVec##aptn); } \
    __force_inline Vector<dtype, len>& normalized() const     		 { temporaryBufferVec##aptn = mem; norm(temporaryBufferVec##aptn); return 			 temporaryBufferVec##aptn;  } \
	__force_inline void                normalize()                   {  norm(mem); return; } \
\
\
    __force_inline dtype&       operator[](uint8_t idx)       { return mem[idx]; } \
    __force_inline const dtype& operator[](uint8_t idx) const { return mem[idx]; } \
		  dtype*        begin()       { return mem.begin();  } \
	      dtype*        end()         { return mem.end();    } \
	const dtype*        begin() const { return mem.begin();  } \
	const dtype*        end()   const { return mem.end();    } \
    constexpr size_t    bytes() const { return mem.bytes();  } \
    __force_inline void print() const { printf(__VA_ARGS__); } \
    \
}; \
\
\
__force_inline Vector<dtype, len>& operator*(dtype a, 			 vec##aptn const& b) { mul       (b.mem, a,         temporaryBufferVec##aptn); return temporaryBufferVec##aptn; } \
__force_inline dtype               dot      (vec##aptn const& a, vec##aptn const& b) { return dot_prod(a.mem, b.mem);       											    } \


#define GENERATE_CROSSPROD_FUNC(len, dtype, aptn) \
__force_inline Vector<dtype, len>& cross    (vec##aptn const& a, vec##aptn const& b) { cross_prod(a.mem, b.mem, temporaryBufferVec##aptn); return temporaryBufferVec##aptn; } \

#define GENERATE_NEGATE_FUNC(len, dtype, aptn) \
__force_inline 		 Vector<dtype, len>& operator-(vec##aptn const& b) { mul(b.mem, ((dtype)-1), temporaryBufferVec##aptn); return temporaryBufferVec##aptn; } \



#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

DEFINE_VECTOR_STRUCTURE_ARGS( \
	2,
	float, 
	2f, 
	struct { float x; float y; }, 
	vec2f(float a, float b) 
	{ 
		x = a; 
		y = b; 
		return; 
	}, 
	"vec2f %p: ( %0.5f, %0.5f )\n", (void*)begin(), x, y
)
GENERATE_NEGATE_FUNC(2, float, 2f)


DEFINE_VECTOR_STRUCTURE_ARGS( \
	4,
	float,
	4f,
	struct { float x; float y; float z; float w; }; 
	struct { float r; float g; float b; float a; },
	vec4f(float a, float b, float c, float d) 
	{ 
		x = a; 
		y = b; 
		z = c; 
		w = d;
		return; 
	},
	"vec4f %p: ( %.05f, %.05f, %.05f, %.05f )\n", (void*)begin(), x, y, z, w
)
GENERATE_NEGATE_FUNC(4, float, 4f)


DEFINE_VECTOR_STRUCTURE_ARGS( \
	3,
	float,
	3f,
	struct { float x; float y; float z; }; 
	struct { float r; float g; float b; };
	vec4f homogenised,
	vec3f(float a, float b, float c) 
	{ 
		x = a; 
		y = b; 
		z = c;
		homogenised.w = 1.0f;
		return; 
	},
	"vec3f %p: ( %.05f, %.05f, %.05f )\n", (void*)begin(), x, y, z
)
GENERATE_CROSSPROD_FUNC(3, float, 3f)
GENERATE_NEGATE_FUNC(3, float, 3f)


struct mat4f 
{
	using underlying_buffer = Vector<float, 16>;
	union {
		Vector<float, 16> mem;
		struct { vec4f row[4];  };
		struct { vec3f row3[4]; };
		float   x0, y0, z0, w0,
				x1, y1, z1, w1, 
				x2, y2, z2, w2, 
				x3, y3, z3, w3;
	};


	mat4f() : mem() 
	{ 
		/* Identity Matrix initialization is pretty useful. */
		mem[ 0] = 1.0f; 
		mem[ 5] = 1.0f; 
		mem[10] = 1.0f; 
		mem[15] = 1.0f; 
		return;
	}
	mat4f(float value) : mem(value) {}
	mat4f(
		float x0, float y0, float z0, float w0, 
		float x1, float y1, float z1, float w1, 
		float x2, float y2, float z2, float w2, 
		float x3, float y3, float z3, float w3
	) {
		row[0] = { x0, y0, z0, w0 }; 
		row[1] = { x1, y1, z1, w1 }; 
		row[2] = { x2, y2, z2, w2 };
		row[3] = { x3, y3, z3, w3 };
		return;
	}
	mat4f(
		vec4f const& a, 
		vec4f const& b,
		vec4f const& c, 
		vec4f const& d
	) {
		row[0] = a;
		row[1] = b;
		row[2] = c;
		row[3] = d;
		return;
	}
	mat4f(const float* validAddr) {
		memcpy(mem.begin(), validAddr, mem.bytes());
		return;
	}
	mat4f(std::array<float, 16> const& arr) : mat4f(arr.begin()) {}
	mat4f(Vector<float, 16>     const& vec) : mem(vec)           {}
	mat4f(mat4f 			    const& cpy) : mem(cpy.mem) 	     {}
	mat4f& operator=(const mat4f& cpy) {
		memcpy(begin(), cpy.begin(), bytes());
		return *this;
	}


	__force_inline underlying_buffer& operator+(mat4f const& b) { add(mem, b.mem, temporaryBufferMat4f); return temporaryBufferMat4f; }
	__force_inline underlying_buffer& operator-(mat4f const& b) { sub(mem, b.mem, temporaryBufferMat4f); return temporaryBufferMat4f; }
	__force_inline underlying_buffer& operator*(mat4f const& b) { mul(mem, b.mem, temporaryBufferMat4f); return temporaryBufferMat4f; }
	__force_inline underlying_buffer& operator*(float        b) { mul(mem, b,     temporaryBufferMat4f); return temporaryBufferMat4f; }
	__force_inline underlying_buffer& operator/(float        b) { div(mem, b,     temporaryBufferMat4f); return temporaryBufferMat4f; }

	__force_inline 		 vec4f  column(	   uint8_t idx		   ) 	   { ifcrashdbg(idx >= 4); return { row[0][idx], row[1][idx], row[2][idx], row[3][idx] }; }
	__force_inline const vec4f  column(	   uint8_t idx		   ) const { ifcrashdbg(idx >= 4); return { row[0][idx], row[1][idx], row[2][idx], row[3][idx] }; }
	__force_inline       vec4f& operator[](uint8_t idx	       )       { ifcrashdbg(idx >= 4); return row[idx];       }
	__force_inline const vec4f& operator[](uint8_t idx	       ) const { ifcrashdbg(idx >= 4); return row[idx];       }
	__force_inline float&       operator()(uint8_t i, uint8_t j)       { 					   return mem[i * 4 + j]; }
	__force_inline const float& operator()(uint8_t i, uint8_t j) const { 					   return mem[i * 4 + j]; }

		  float*        begin()        { return mem.begin();  }
		  float*        end()          { return mem.end();    }
	const float*        begin()  const { return mem.begin();  }
	const float*        end()    const { return mem.end();    }
	constexpr size_t    bytes()  const { return mem.bytes();  }
	constexpr size_t    length() const { return mem.len();    }
	__force_inline void print()  const 
	{ 
		printf("mat4f %p:\n", (void*)begin());
		printf("( %.05f, %.05f, %.05f, %.05f )\n", row[0].x, row[0].y, row[0].z, row[0].w);
		printf("( %.05f, %.05f, %.05f, %.05f )\n", row[1].x, row[1].y, row[1].z, row[1].w);
		printf("( %.05f, %.05f, %.05f, %.05f )\n", row[2].x, row[2].y, row[2].z, row[2].w);
		printf("( %.05f, %.05f, %.05f, %.05f )\n", row[3].x, row[3].y, row[3].z, row[3].w);
		return;
	}
};


__force_inline mat4f::underlying_buffer& operator*(float a, mat4f const& b) { mul(b.mem, a, temporaryBufferMat4f); return temporaryBufferMat4f; }


void MultiplyMat4Vec4(vec4f& a, mat4f& b, vec4f& out);
void MultiplyMat4Mat4(mat4f& a, mat4f& b, mat4f& out);


void identity  (					    mat4f& out);
void translate (vec3f const& translate, mat4f& out);
void scale     (vec3f const& scale,     mat4f& out);
void transposed(mat4f const& in, 		mat4f& out);
void transpose (					  mat4f& inout);


/* 
	Explanation & Derivation:
	https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/opengl-perspective-projection-matrix
*/
void perspective(
	float  aspectRatio, 
	float  fovy, 
	float  near, 
	float  far, 
	mat4f& out
);


/* 
	Calculated formula using 4x4 inverse formula + some regex + python str replace:
	where m%d%d = projection matrix with %d as subscripts (indices).
	{
		1/m00,   0,     0,          0,
		0,     1/m11,   0,          0,
		0,       0,     0,        1/m32,
		0,       0,   1/m23, -m22/(m23 * m32)
	}
*/
void inv_perspective(mat4f const& in, mat4f& out);



/* 
	Using a Right Handed Coordinate System (+z towards viewer, +x is right, +y is up)
	This Function is the World->Camera Transform Matrix.

	given:
	V_world = V_cam * X, where X = Camera's Transform Matrix, Or Camera_Space->World_Space
	Because we want V_cam =>

	V_world * X^-1 = V_cam.

	In Row Major Convention, X = R * T, where T = translation, R = rotation across each basis in 3d.
		==> X^-1 = (R * T)^-1 = T^-1 * R^-1
		<==> View_Matrix = T^-1 * R^-1 = translate(-eyePos) * BasisVectorMatrix (RotateXYZ, ...)
	This link will help - https://stackoverflow.com/questions/349050/calculating-a-lookat-matrix?noredirect=1&lq=1
	NOTE ABOUT LINK CONTENTS: The Matrix is left Handed (At - eye instead of eye - At), but the math is the same.
	
	Resulting View Matrix (World->Camera):
		where xAxis = right, yAxis = newup, zAxis = forward
		[         xaxis.x          yaxis.x          zaxis.x  0 ]
		[         xaxis.y          yaxis.y          zaxis.y  0 ]
		[         xaxis.z          yaxis.z          zaxis.z  0 ]
		[ dot(xaxis,-eye)  dot(yaxis,-eye)  dot(zaxis,-eye)  1 ]
*/
void lookAt(
	vec3f const& eyePos, 
	vec3f const& at, 
	vec3f const& up,
	mat4f& 		 out
);


/* 
	Inverse of a homogenous transform matrix (V) =
		[ xaxis.x  xaxis.y  xaxis.z  eye.x ]
		[ yaxis.x  yaxis.y  yaxis.z  eye.y ]
		[ zaxis.x  zaxis.y  zaxis.z  eye.z ]
		[    0 	      0 	   0       1   ]
*/
void inv_lookAt(
	vec3f const& eyePos, 
	vec3f const& at, 
	vec3f const& up,
	mat4f& 		 out
);


/* 
	Source Code: https://github.com/willnode/N-Matrix-Programmer/blob/master/Info/Matrix_4x4.txt
*/
void inverse(
	mat4f const& in,
	mat4f&       out
);

#pragma GCC diagnostic pop


using point2 = vec2f;
using point3 = vec3f;
using point4 = vec4f;




NAMESPACE_MATH_END