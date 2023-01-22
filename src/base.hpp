#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <atomic>


#ifndef aligned_alloc
	#define aligned_alloc(align, size) _aligned_malloc(size, align)
	#define aligned_free(ptr) _aligned_free(ptr)
#endif



#define ifcrash(condition) \
	if(!!(condition)) { \
		fprintf(stderr, "File %s Line %u\n", __FILE__, __LINE__); \
		throw std::runtime_error("An 'ifcrash' check statement was triggered"); \
	} \
	\
\


extern uint8_t run_once;
#define __once(code_block) \
	if(!boolean(run_once)) { \
		{ \
			code_block; \
			++run_once; \
		} \
	} \

#define RESET_RUN_ONCE_COUNT() run_once = 0;


extern uint16_t finished;
#define run_block(times, code_block) \
	if(boolean((uint16_t)times - finished)) { \
		{ \
			code_block; \
			++finished; \
		} \
	} \

#define RESET_RUN_TIMES_COUNT finished = 0;




#ifdef _DEBUG
extern std::atomic<size_t> markflag;


#define debug(...) { __VA_ARGS__; }
#define debugnobr(...) __VA_ARGS__;

#define ifcrashmsg(condition, str, ...) \
	if(!!(condition)) { \
		fprintf(stderr, "File %s Line %u\n    Message: \n", __FILE__, __LINE__); \
		fprintf(stderr, str, __VA_ARGS__); \
		throw std::runtime_error("An 'ifcrashmsg' check statement was triggered"); \
	} \
	\
\

#define ifcrashdbg(condition) \
	if(!!(condition)) { \
		fprintf(stderr, "File %s Line %u\n", __FILE__, __LINE__); \
		throw std::runtime_error("An 'ifcrashdebug' check statement was triggered"); \
	} \
	\
\

#define ifcrashdo(condition, action) \
	if(!!(condition)) { \
		fprintf(stderr, "File %s Line %u\n    Message: \n", __FILE__, __LINE__); \
		{ action; } \
		throw std::runtime_error("An 'ifcrashmsg' check statement was triggered"); \
	} \
\

#define ifcrashmsg_do(condition, action, str, ...) \
	if(!!(condition)) { \
		fprintf(stderr, "File %s Line %u\n    Message: \n", __FILE__, __LINE__); \
		fprintf(stderr, str, __VA_ARGS__); \
		{ action; } \
		throw std::runtime_error("An 'ifcrashmsg' check statement was triggered"); \
	} \
	\
\

#else
#define debug(...)
#define debugnobr(...)
#define ifcrashmsg(condition, str, ...) {}
#define ifcrashdbg(condition) 			{}
#define ifcrashdo(condition, action) ifcrash(condition);
#define ifcrashmsg_do(condition, action, str, ...) {}

#endif



#ifdef _DEBUG

#define mark() \
	{ printf("mark %lu here: %s line %u\n", markflag.load(), __FILE__, __LINE__); ++markflag; } \


#define markstr(str) \
	{ printf("mark %lu here: %s line %u extra: %s\n", markflag.load(), __FILE__, __LINE__, str); ++markflag; } \


#define markfmt(str, ...) \
	{ \
		printf("mark %lu here: %s line %u extra: ", markflag.load(), __FILE__, __LINE__); \
		printf(str, __VA_ARGS__); \
		++markflag; \
	} \


#else

#define mark() 		 	  do {} while(0);
#define markstr(str) 	  do {} while(0);
#define markfmt(str, ...) do {} while(0);


#endif

/* align_malloc not really correct, should alloc size+alignment then return the ptr aligned */
#define align_malloc(size, alignment) malloc(  alignment * ( (size/alignment) + boolean(size%alignment) )  )
#define amalloc_t(type, size, align) (type*)aligned_alloc(align, size)


#define boolean(arg) !!(arg)
#define KB           	 (1024ul)
#define MB           	 (KB*KB)
#define GB           	 (MB*MB)
#define PAGE         	 (4 * KB)
#define CACHE_LINE_BYTES (64ul)
#define DEFAULT8         (0xAA)
#define DEFAULT16        (0xF00D)
#define DEFAULT32        (0xBABEBABE)
#define DEFAULT64        (0xFACADE00FACADE00)
#define DEFAULT128       (0xBEBC0FFEEAC1DBEB)                        

#if defined( __GNUC__ ) || defined( __MINGW__ ) || defined ( __clang__ )
#define 	likely(cond)    __builtin_expect( boolean(cond), 1 )
#define 	unlikely(cond)  __builtin_expect( boolean(cond), 0 )
#elif defined( _MSC_VER )
#define 	likely(cond)
#define 	unlikely(cond)
#else
#   warning "Unknown compiler, there might be some troubles during compilation, such as undefined macros.
#endif


#define __hot           __attribute__((hot))
#define __cold          __attribute__((cold))
#ifndef __unused
#define __unused        __attribute__((unused)) /* more appropriate for functions		    */
#endif
#define notused         __attribute__((unused)) /* more appropriate for function parameters */
#define pack            __attribute__((packed))
#define lin(size)       __attribute__((aligned(size)))
#define alignpack(size) __attribute__((packed, aligned(size)))
#define alignsz(size)   __attribute__((aligned(size)))

#ifndef __force_inline 
#define __force_inline inline __attribute__((always_inline))
#else
#define __force_inline __always_inline
#endif



typedef char char_t;
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t  i64;
typedef int32_t  i32;
typedef int16_t  i16;
typedef int8_t   i8;
typedef float    f32;
typedef double   f64;