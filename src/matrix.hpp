#pragma once
#include "base.hpp"
#include <vector>
#include <stdio.h>




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


	constexpr size_t length() const { return Length; 	   }
			  T* 	 data()   const { return this->__data; }
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




template<typename T, u16 RowLength, bool Lower = true> struct TriangularMatrix : public Vector<T, __scast(u32, RowLength) * RowLength / 2>
{
private:
	/* 
		Since memory is ordered in a0, a1 a2, a3 a4 a5, ..., aN (Or Reversed For Upper, meaning starting from N elements and going down to A[n, n]),
		We need a function that will translate the linear indices to indices in the new memory layout, in a way that will preserve the original mapping from element-to-element.
	*/
	constexpr u32 linearToLowerTriangularIdx(u32 i, u32 j) { return j + (i + 1) * i/2; }
	constexpr u32 linearToUpperTriangularIdx(u32 i, u32 j) { return j + (2*RowLength + 1 - i) * i/2; }


public:
	TriangularMatrix() { 
		this->__data = nullptr; 
	}
	

	explicit TriangularMatrix(T* to_use) {
		this->__data = to_use;
		return;
	}


	TriangularMatrix(const_ref<T> initVal) {
		ifcrashdbg(this->allocate());
		
		for(u32 i = 0; i < this->length(); ++i) {
			this->__data[i] = initVal;
		}
		return;
	}


	TriangularMatrix(TriangularMatrix& cpy) {
		this->allocate();
		this->copy(cpy.__data);
		return;
	}


	TriangularMatrix& operator=(TriangularMatrix& cpy) { 
		if(likely(this->__data == nullptr)) 
			this->allocate();

		this->copy(cpy.__data);
		return *this;
	}


	TriangularMatrix& operator=(TriangularMatrix&& mov) {
		ifcrashmsg(this->__data != nullptr, "You shouldn't be moving Data to objects that are already initialized.");
		// if(unlikely(this->__data != nullptr)) 
		// 	this->destroy();
		this->__data = mov.__data; 
		return *this; 
	}


	~TriangularMatrix() { this->destroy(); return; }


		  T& operator()(u16 i, u16 j) 	    { if constexpr(Lower) return this->__data[linearToLowerTriangularIdx(i, j)]; else return this->__data[linearToUpperTriangularIdx(i, j)]; }
	const T& operator()(u16 i, u16 j) const { if constexpr(Lower) return this->__data[linearToLowerTriangularIdx(i, j)]; else return this->__data[linearToUpperTriangularIdx(i, j)]; }


	Vector<T, RowLength> operator*(Vector<T, RowLength> const& v)
	{
		Vector<T, RowLength> out{T{}};
		for(size_t i = 0; i < RowLength; ++i) {
			out[i] = dot(this->operator[](i), v);
		}
		return out;
	}


};


// template<typename T, u16 RowLength> struct SparseMatrix
// {
// private:
// 	T* __data;
// 	std::vector<u32> IndexJ;
// 	std::vector<u32> IndexI;
// 	u32 nnz;
// 	u32 lastI;


// 	bool allocate() {
// 		size_t alloc_size = (sizeof(T) * nnz);
// 		if constexpr (sizeof(T) % 2 == 0) {
// 			__data = amalloc_t(T, alloc_size, sizeof(T)); 
// 		} else {
// 			__data = malloc(alloc_size);
// 		}

// 		IndexJ.resize(nnz);
// 		IndexI.resize(RowLength + 1);
// 		return __data == nullptr;
// 	}


// 	void destroy() {
// 		if constexpr (sizeof(T) % 2 == 0) { 
// 			afree_t(__data);
// 		} 
// 		else { 
// 			free(__data);
// 		}
// 		__data = nullptr;
// 		return;
// 	}


// 	void copy(T* other) { memcpy(__data, other, (sizeof(T) * nnz)); return; }


// 	/*
// 		Rewrite this based on: https://www.youtube.com/watch?v=a2LXVFmGH_Q (Wikipedia poopoo)
// 	*/
// 	void construct(Matrix<T, RowLength> M)
// 	{
// 		constexpr size_t 
// 			shrink = 8 * sizeof(u64), 
// 			RowDiv = RowLength / shrink, 
// 			RowMod = RowLength % shrink;

// 		constexpr size_t bitmapRow = RowDiv + boolean(RowMod);


// 		std::array<u64, bitmapRow * RowLength> bitmap; /* Construct Bitmap for all non-zeros. */
// 		size_t j = 0, i = 0;
// 		u32 nz = 0, nzAccum = 0, push = 0;
// 		u64 eqz = 0, eightElems = 0, packed_bits = 0;


// 		for(size_t bm_i = 0; bm_i < RowLength; ++bm_i) {

// 			for(size_t bm_j = 0; bm_j < bitmapRow - 1; ++bm_j) 
// 			{ /* All elements from 0 -> bitmapRow - 1 represent 64 elements. the last one represents (RowLength % 64) elements */
// 				eqz = 0;
// 				/* Reducing 64 Elements to a single u64 bitmap */
// 				for(u8 boolIdx = 0; boolIdx < 8; ++boolIdx) {
// 					eightElems = 0;
// 					eightElems |= boolean( M.data()[i + 8 * boolIdx + 0] != ((T)0) ) << 0;
// 					eightElems |= boolean( M.data()[i + 8 * boolIdx + 1] != ((T)0) ) << 1;
// 					eightElems |= boolean( M.data()[i + 8 * boolIdx + 2] != ((T)0) ) << 2;
// 					eightElems |= boolean( M.data()[i + 8 * boolIdx + 3] != ((T)0) ) << 3;
// 					eightElems |= boolean( M.data()[i + 8 * boolIdx + 4] != ((T)0) ) << 4;
// 					eightElems |= boolean( M.data()[i + 8 * boolIdx + 5] != ((T)0) ) << 5;
// 					eightElems |= boolean( M.data()[i + 8 * boolIdx + 6] != ((T)0) ) << 6;
// 					eightElems |= boolean( M.data()[i + 8 * boolIdx + 7] != ((T)0) ) << 7;
// 					eqz |= ( eightElems << (8 * boolIdx) );
// 				}
// 				bitmap[bm_j + bm_i * bitmapRow] = eqz;
// 				i += shrink;
// 			}
			
// 			eightElems = 0; /* Complete the remainder of the elements. Might actually be also 64 if RowLength is divisible but oh well */
// 			for(size_t r = 0; r < RowMod; ++r) 
// 			{
// 				eightElems |= boolean( M.data()[i + r] != ((T)0) ) << r;
// 			}
// 			bitmap[bitmapRow - 1 + bm_i * bitmapRow] = eightElems;
// 			i += RowMod;
// 		}


// 		/* 
// 			Size of IndexJ, __data is the amount of Non-Zero Elements in M.
// 			Afterwards We !!Allocate Memory!!
// 		*/
// 		for(auto& notZero64 : bitmap) { nz += __builtin_popcountll(notZero64); }
// 		nnz = nz;
// 		allocate();
// 		IndexI[0] = 0;


// 		/* Construct IndexI */
// 		/* 
// 			In the current 
// 		*/
// 		nz = 0;
// 		for(i = 0; i < RowLength; ++i) 
// 		{
// 			u8 idx = 0;
// 			for(j = 0; j < bitmapRow; ++j) {
// 				packed_bits = bitmap[j + i * bitmapRow];
// 				while(packed_bits) {
// 					idx = __builtin_ffsll(packed_bits); /* retrieve index of lsb [find first non-zero element INDEX]   */
// 					packed_bits &= ~(1 << idx); 	    /* turn it off for next iter */

// 					++nz; 								/* found another non-zero */
// 					IndexJ.push_back(j); 			/* push its column-index  */
// 					__data[push] = M.data()[(i * bitmapRow + j) * shrink + idx]; /* push value to __data, 3d->1d array access, where dims are (bitmapRow, RowLength, 64) */
// 					++push;
// 				}
// 			}

// 			nzAccum += nz;
// 			nz = 0;
// 			IndexI.push_back(nzAccum);
// 		}

// 		lastI = IndexI.back();
// 		return;
// 	}


// public:
// 	SparseMatrix(): __data{nullptr}, IndexJ(), IndexI() {}
// 	SparseMatrix(Matrix<T, RowLength> matrix) 
// 	{
// 		ifcrashdbg(allocate());
// 		construct(matrix);
// 		return;
// 	}


// 	SparseMatrix(SparseMatrix& cpy) 
// 	{
// 		allocate();
// 		copy(cpy.__data);
// 		return;
// 	}


// 	SparseMatrix& operator=(SparseMatrix& cpy)  
// 	{ 
// 		if(likely(__data == nullptr)) 
// 			allocate();

// 		copy(cpy.__data);
// 		return *this;
// 	}


// 	SparseMatrix& operator=(SparseMatrix&& mov) 
// 	{
// 		ifcrashmsg(__data != nullptr, "You shouldn't be moving Data to objects that are already initialized.");
// 		__data = mov.__data;
// 		return *this; 
// 	}


// 	~SparseMatrix() { destroy(); return; }


// 	T operator()(u16 i, u16 j) 	    
// 	{
// 		// if(i > lastI )
// 	}
// 	const T operator()(u16 i, u16 j) const 
// 	{

// 	}
	
	
// 	void set(u16 i, u16 j)
// 	{

// 	}


// 	T* data() const { return __data; }


// };




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