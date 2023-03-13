#pragma once
#include "base.hpp"




template<typename T, uint32_t Length> struct Vector {
private:
	T* __data;


	bool allocate() {
		size_t alloc_size = (sizeof(T) * Length);
		if constexpr (sizeof(T) % 2 == 0) {
			__data = amalloc_t(T, alloc_size, sizeof(T)); 
		} else {
			__data = malloc(alloc_size);
		}
		return __data == nullptr;
	}


	void destroy() {
		if constexpr (sizeof(T) % 2 == 0) { 
			afree_t(__data);
		} 
		else { 
			free(__data);
		}
		__data == nullptr;
		return;
	}


	void copy(T* other) {
		for(size_t i = 0; i < Length; ++i) {
			__data[i] = other[i];
		}
		return;
	}

public:
	Vector(): __data{nullptr} {}
	
	
	explicit Vector(T* to_use) : __data{to_use} {}


	explicit Vector(const_ref<T> initVal) 
	{
		ifcrashdbg(allocate());
		
		for(u32 i = 0; i < Length; ++i) {
			__data[i] = initVal;
		}
		return;
	}


	Vector(Vector& cpy) 
	{
		allocate();
		copy(cpy.__data);
		return;
	}


	Vector& operator=(Vector& cpy)  
	{ 
		if(__data) { destroy(); }

		allocate(); 
		copy(cpy.__data);
		return *this;
	}


	Vector& operator=(Vector&& mov) 
	{ 
		__data = mov.__data; 
		return *this; 
	}


	~Vector() { destroy(); return; }



	Vector operator-(Vector const& B) { Vector tmp{T{}}; for(size_t i = 0; i < Length; ++i) { tmp[i] = __data[i] - B[i]; } return tmp; }
	Vector operator+(Vector const& B) { Vector tmp{T{}}; for(size_t i = 0; i < Length; ++i) { tmp[i] = __data[i] + B[i]; } return tmp; }
	Vector operator*(Vector const& B) { Vector tmp{T{}}; for(size_t i = 0; i < Length; ++i) { tmp[i] = __data[i] * B[i]; } return tmp; }
	Vector operator-(const_ref<T>  b) { Vector tmp{T{}}; for(size_t i = 0; i < Length; ++i) { tmp[i] = __data[i] - b;    } return tmp; }
	Vector operator+(const_ref<T>  b) { Vector tmp{T{}}; for(size_t i = 0; i < Length; ++i) { tmp[i] = __data[i] + b;    } return tmp; }
	Vector operator*(const_ref<T>  b) { Vector tmp{T{}}; for(size_t i = 0; i < Length; ++i) { tmp[i] = __data[i] * b;    } return tmp; }
	
	Vector& operator-=(Vector const& B) { for(size_t i = 0; i < Length; ++i) { __data[i] -= B[i]; } return *this; }
	Vector& operator+=(Vector const& B) { for(size_t i = 0; i < Length; ++i) { __data[i] += B[i]; } return *this; }
	Vector& operator*=(Vector const& B) { for(size_t i = 0; i < Length; ++i) { __data[i] *= B[i]; } return *this; }
	Vector& operator-=(const_ref<T>  b) { for(size_t i = 0; i < Length; ++i) { __data[i] -= b;    } return *this; }
	Vector& operator+=(const_ref<T>  b) { for(size_t i = 0; i < Length; ++i) { __data[i] += b;    } return *this; }
	Vector& operator*=(const_ref<T>  b) { for(size_t i = 0; i < Length; ++i) { __data[i] *= b;    } return *this; }
	

	static T dot(Vector const& A, Vector const& B) {
		T tmp{};
		for(size_t i = 0; i < Length; ++i) { tmp += A[i] * B[i]; }
		return tmp; 
	}


	T magnitudeSquared() const { return dot(*this, *this); 		   }
	T magnitude() 		 const { return std::sqrt<T>(magnitudeSquared()); }


		  T& operator[](u16 idx) 	   { return __data[idx]; }
	const T& operator[](u16 idx) const { return __data[idx]; }
		  T& at(u16 idx) 	   { ifcrashdbg(idx >= Length); return __data[idx]; }
	const T& at(u16 idx) const { ifcrashdbg(idx >= Length); return __data[idx]; }
};


template<typename T, uint16_t Length> Vector<T, Length> operator*(const_ref<T> val, Vector<T, Length> vec) {
	return (vec * val);
}




template<typename T, uint16_t RowLength> struct Matrix : public Vector<T, __scast(u32, RowLength) * RowLength> {
		  Vector<T, RowLength> operator[](u16 idx) 			  { return Vector<T, RowLength>(&this->__data[idx]); }
	const Vector<T, RowLength> operator[](u16 idx) 		const { return Vector<T, RowLength>(&this->__data[idx]); }
		  T& 				   operator()(u16 i, u16 j) 	  { return this->__data[j + i * RowLength]; }
	const T& 				   operator()(u16 i, u16 j) const { return this->__data[j + i * RowLength]; }


	Vector<T, RowLength> operator*(Vector<T, RowLength> const& v)
	{
		Vector<T, RowLength> out{T{}};
		for(size_t i = 0; i < RowLength; ++i) {
			out[i] = Vector<T, RowLength>::dot(this->operator[](i), v);
		}
		return out;
	}
};




template<uint16_t SideLengthN> using Matrixf = Matrix<f32, SideLengthN>;
template<uint16_t SideLengthN> using Matrixd = Matrix<f64, SideLengthN>;
template<uint16_t SideLengthN> using Matrixi = Matrix<i32, SideLengthN>;
template<uint16_t Length>      using Vectorf = Vector<f32, Length>;
template<uint16_t Length>      using Vectord = Vector<f64, Length>;
template<uint16_t Length>      using Vectori = Vector<i32, Length>;