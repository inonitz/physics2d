#pragma once
#include "base.hpp"
#include <cstring>
#include <functional>



/* 
	* Only 2 Restrictions with this class:
		* If you allocated 5 elements but try to copy 7 into them using copy(), 
		* be prepared for undefined behaviour.
		* The ONLY guaranteed behaviour is copying to an uninitialized manvec, 
		* in which the buffer will be automatically allocated to fit the request.
		* This is done purely for performance, nothing else.
	
	* Copy Function NOTE:
		bufA.copy(bufB);
		Conditions: 
			bufA == nullptr: bufA implicitly created then copied from bufB.
			bufA != nullptr: (bufA.size >= bufB.size) condition MUST apply
*/
template<typename DataType>
class manvec 
{
private:
	typedef DataType* 		iterator;
	typedef const DataType* const_iterator;
	
	
	DataType* ptr;
	size_t    size;

	constexpr size_t underlying_size()      const { return sizeof(DataType*) + sizeof(size_t); }
			  size_t arraySize() 	        const { return sizeof(DataType) * size; 		   }
	constexpr size_t underlying_alignment() const { 
		// if sizeof(DataType) isn't power of 2: 
		//     Use CACHE_LINE_BYTES (64 Bytes on most modern processors) 
		// else Use the size of the element
		#define IS_DATATYPE_SIZE_POWER_OF_2 boolean(  (sizeof(DataType) != 0) && ( (sizeof(DataType) & (sizeof(DataType) - 1) ) == 0)  )
			return IS_DATATYPE_SIZE_POWER_OF_2 * sizeof(DataType) + !IS_DATATYPE_SIZE_POWER_OF_2 * CACHE_LINE_BYTES;
		#undef IS_DATATYPE_SIZE_POWER_OF_2
	}

public:
	manvec() : ptr(nullptr), size(0) {}

	explicit manvec(DataType* buffer, size_t length): ptr(buffer), size(length) {}	
	explicit manvec(size_t start_size) 
	{
		ifcrash(start_size == 0);
		
		size = start_size;
		ptr  = (DataType*)_mm_malloc(size * sizeof(DataType), underlying_alignment());
		
		ifcrash(ptr == nullptr);
	}


	manvec(std::initializer_list<DataType>&& list)
	{
		if(list.size() == 0) { 
			ptr = nullptr;
			return; 
		}
		ptr  = (DataType*)_mm_malloc(size * sizeof(DataType), underlying_alignment());
		size = list.size();
		memcpy((void*)ptr, (void*)list.begin(), arraySize()); /* redundant copy, but C++ is stupid... */
	}


	~manvec() {
		_mm_free(ptr); 
		ptr  = nullptr;
		size = 0;
	}
	

	manvec(const manvec& copy) 			  = delete;
	manvec& operator=(const manvec& copy) = delete;


	manvec(manvec&& mov) : ptr(mov.ptr), size(mov.size) {}

	manvec& operator=(manvec&& mov)
	{
		ptr  = mov.ptr;
		size = mov.size;
		return *this;
	}


	const_iterator   begin() const  { return ptr;         }
		  iterator   begin() 	    { return ptr;         }
	const_iterator   end()   const  { return &ptr[size];  }
		  iterator   end()          { return &ptr[size];  }
	constexpr size_t len()   const  { return size;        }
	constexpr size_t len()          { return size;        }
	constexpr size_t bytes() const  { return arraySize(); }
	bool 			 empty() const  { return size == 0 || ptr == nullptr; }

	DataType& operator[](size_t index)
	{
		ifcrashdbg(index >= size);
		return ptr[index];
	}

	const DataType& operator[](size_t index) const
	{
		ifcrashdbg(index >= size);
		return ptr[index];
	}
	

	void copy(manvec& from)
	{
		if(!ptr) {
			size = from.size;
			ptr  = (DataType*)_mm_malloc(size * sizeof(DataType), underlying_alignment());
		}
		memcpy(ptr, from.ptr, from.bytes());

		return;
	}


	void copy(const manvec& from)
	{
		if(!ptr) {
			size = from.size;
			ptr  = (DataType*)_mm_malloc(size * sizeof(DataType), underlying_alignment());
		}
		memcpy(ptr, from.ptr, from.bytes());
		return;
	}


	void copy(DataType* raw, size_t elements)
	{
		if(!ptr) {
			size = elements;
			ptr  = (DataType*)_mm_malloc(size * sizeof(DataType), underlying_alignment());
		}
		memcpy(ptr, raw, elements * sizeof(DataType));
		return;
	}


	void copy(DataType const* raw, size_t elements)
	{
		if(!ptr) {
			size = elements;
			ptr  = (DataType*)_mm_malloc(size * sizeof(DataType), underlying_alignment());
		}
		memcpy(ptr, raw, elements * sizeof(DataType));

		return;
	}

	template<size_t bufferSize>
	void copy(std::array<DataType, bufferSize>& from)
	{
		if(!ptr) {
			size = from.size();
			ptr  = _mm_malloc(size * sizeof(DataType), underlying_alignment());
		}
		memcpy(ptr, from.data(), bufferSize * sizeof(DataType));
	}


	void increase(size_t elements)
	{
		if(!elements) return;
		size_t    tmp    = size + elements;
		DataType* newBuf = (DataType*)_mm_malloc(size * sizeof(DataType), underlying_alignment());

		if(ptr) {
			memcpy((void*)newBuf, (void*)ptr, bytes()); 
			_mm_free(ptr);
		}
		ptr  = newBuf;
		size = tmp;
		return;
	}


	void increaseTo(size_t elements) {
		ifcrashdbg(!elements || elements <= size) return;
		size_t    newSize = elements;
		DataType* newBuf  = (DataType*)_mm_malloc(size * sizeof(DataType), underlying_alignment());

		if(ptr) {
			memcpy((void*)newBuf, (void*)ptr, arraySize()); 
			_mm_free(ptr);
		}
		ptr  = newBuf;
		size = newSize;
	}


	void clear() 
	{
		if(!ptr) return;
		memset(ptr, 0x00, bytes());
		return;
	}


	void print(const std::function<void(DataType&)>& printType)
	{
		printf("manual Vector:\n    Begin: 0x%llX\n    Size : %llu\n    Bytes: 0x%llX\n",
			(uint64_t)begin(),
			size,
			size * sizeof(DataType)
		);
		if(size == 0) return;

		
		printf("    Buffer: [");
		size_t i = 0;
		for(; i < size - 1; ++i)
		{
			printType(ptr[i]);
			printf(", ");
		}
		printType(ptr[i]);
		printf("]\n\n");
	}


	void print()
	{
		printf("manual Vector:\n    Begin: 0x%llX\n    Size : %llu\n    Bytes: 0x%llX\n",
			(size_t)begin(),
			size,
			size * sizeof(DataType)
		);
	}
	
};
