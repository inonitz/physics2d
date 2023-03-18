#pragma once
#include "base.hpp"




template<typename T, uint32_t Length, bool PointerNotAllocated = false> struct Vector {
protected:
	T* __data;


	bool allocate() {
		if constexpr (PointerNotAllocated) 
			return true;


		size_t alloc_size = (sizeof(T) * Length);
		if constexpr (sizeof(T) % 2 == 0) {
			__data = amalloc_t(T, alloc_size, sizeof(T)); 
		} else {
			__data = malloc(alloc_size);
		}
		return __data == nullptr;
	}


	void destroy() {
		if constexpr (PointerNotAllocated) 
			return;


		if constexpr (sizeof(T) % 2 == 0) { 
			afree_t(__data);
		} 
		else { 
			free(__data);
		}
		__data = nullptr;
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


	Vector(const_ref<T> initVal) 
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
		if(likely(__data == nullptr)) 
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
	

	T magSquared() const { return dot(*this, *this); 		  }
	T mag() 	   const { return std::sqrt<T>(magSquared()); }


		  T& operator[](u16 idx) 	   { return __data[idx]; }
	const T& operator[](u16 idx) const { return __data[idx]; }
		  T& at(u16 idx) 	   { ifcrashdbg(idx >= Length); return __data[idx]; }
	const T& at(u16 idx) const { ifcrashdbg(idx >= Length); return __data[idx]; }


	constexpr size_t length() const { return Length; }
};


template<typename T, uint16_t Length> Vector<T, Length> operator*(const_ref<T> val, Vector<T, Length> vec) {
	return (vec * val);
}


template<typename T, uint16_t Length> T dot(Vector<T, Length> const& A, Vector<T, Length> const& B) {
	T tmp{}; for(size_t i = 0; i < Length; ++i) { tmp += A[i] * B[i]; }
	return tmp; 
}




template<typename T, uint16_t RowLength> struct Matrix : public Vector<T, __scast(u32, RowLength) * RowLength> {
	Matrix() { 
		this->__data = nullptr; 
	}
	

	explicit Matrix(T* to_use) {
		this->__data = to_use;
		return;
	}


	Matrix(const_ref<T> initVal) {
		ifcrashdbg(this->allocate());
		
		for(u32 i = 0; i < this->length(); ++i) {
			this->__data[i] = initVal;
		}
		return;
	}


	Matrix(Matrix& cpy) {
		this->allocate();
		this->copy(cpy.__data);
		return;
	}


	Matrix& operator=(Matrix& cpy) { 
		if(likely(this->__data == nullptr)) 
			this->allocate();

		this->copy(cpy.__data);
		return *this;
	}


	Matrix& operator=(Matrix&& mov) {
		if(unlikely(this->__data != nullptr)) 
			this->destroy();
  
  
		this->__data = mov.__data; 
		return *this; 
	}


	~Matrix() { this->destroy(); return; }


		  Vector<T, RowLength> operator[](u16 idx) 			  { return Vector<T, RowLength, true>(&this->__data[idx]); }
	const Vector<T, RowLength> operator[](u16 idx) 		const { return Vector<T, RowLength, true>(&this->__data[idx]); }
		  T& 				   operator()(u16 i, u16 j) 	  { return this->__data[j + i * RowLength]; }
	const T& 				   operator()(u16 i, u16 j) const { return this->__data[j + i * RowLength]; }


	Vector<T, RowLength> operator*(Vector<T, RowLength> const& v)
	{
		Vector<T, RowLength> out{T{}};
		for(size_t i = 0; i < RowLength; ++i) {
			out[i] = dot(this->operator[](i), v);
		}
		return out;
	}


};


template<typename T, u16 RowLength> void tranpose(Matrix<T, RowLength> const& A, Matrix<T, RowLength>& Out) {
	for(size_t i = 0; i < RowLength; ++i)
	{
		for(size_t j = 0; j < RowLength; ++j) {
			Out(i, j) = A(j, i);
		}
	}
	return;
}


template<typename T, u16 RowLength> void matmul(Matrix<T, RowLength> const& A, Matrix<T, RowLength> const& B, Matrix<T, RowLength>& Out)
{
	tranpose(B, Out);
	for(size_t i = 0; i < RowLength; ++i) {
		Out[i] = A * Out[i];
	}
	return;
}



template<u16 SideLengthN> using Matrixf = Matrix<f32, SideLengthN>;
template<u16 SideLengthN> using Matrixd = Matrix<f64, SideLengthN>;
template<u16 SideLengthN> using Matrixi = Matrix<i32, SideLengthN>;
template<u16 Length>      using Vectorf = Vector<f32, Length>;
template<u16 Length>      using Vectord = Vector<f64, Length>;
template<u16 Length>      using Vectori = Vector<i32, Length>;
template<u16 Length>      using VectorfView = Vector<f32, Length, true>;
template<u16 Length>      using VectordView = Vector<f64, Length, true>;
template<u16 Length>      using VectoriView = Vector<i32, Length, true>;