#pragma once

#if defined(_WIN32)
  #define ON_PLATFORM_WIN32 1
#endif

#include <assert.h>
#include <memory.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <threads.h>
#include <uchar.h>
#include <wchar.h>

#if defined(ON_PLATFORM_WIN32)
  #include <tchar.h>
  #include <Windows.h>

  #pragma comment(lib, "User32.lib")
#endif

#if !defined(__FUNCTION__)
  #if defined(__func__)
    #define __FUNCTION__ __func__
  #endif
#endif

#define ASSUME(...) __assume(__VA_ARGS__) 
#define UNREACHABLE() ASSUME(0)
#define UNIMPLEMENTED() do { assert(!"unimplemented"); UNREACHABLE(); } while (0);

/*****************************************************************************/

int start(int arguments_count, char **arguments);

/*****************************************************************************/

#define alignof(...) _Alignof(__VA_ARGS__)
#define alignas(...) _Alignas(__VA_ARGS__)
#define countof(x) (sizeof(x) / sizeof(x[0]))

/*****************************************************************************/

#define print_comment(...) printf("[comment] " __VA_ARGS__)
#define print_caution(...) printf("[caution] " __VA_ARGS__)
#define print_failure(...) printf("[failure] " __VA_ARGS__)

/*****************************************************************************/

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef uint8  uintb;
typedef uint16 uints;
typedef uint32 uint;
typedef uint64 uintl;

#define uintb_bits_count ((uint)8)
#define uints_bits_count ((uint)16)
#define uint_bits_count  ((uint)32)
#define uintl_bits_count ((uint)64)

#define uint_maximum_value (~(uint)0)

typedef int8_t  sint8;
typedef int16_t sint16;
typedef int32_t sint32;
typedef int64_t sint64;

typedef sint8  sintb;
typedef sint16 sints;
typedef sint32 sint;
typedef sint64 sintl;

typedef float  float32;
typedef double float64;

typedef float64 floatl;

typedef uint8 byte;
typedef uint8 bit;

#define byte_width ((uint)8)

uintb clz(uintl value);

uintb ctz(uintl value);

sintl index_bit_range(uintb range_size, bit of_zeros, const uint *bytes, uint bytes_count);

sintl toggle_bit_range(uintb range_size, bit of_zeros, uint *bytes, uint bytes_count);

#define lmask1  ((uint)0x00000001)
#define lmask2  ((uint)0x00000003)
#define lmask3  ((uint)0x00000007)
#define lmask4  ((uint)0x0000000f)
#define lmask5  ((uint)0x0000001f)
#define lmask6  ((uint)0x0000003f)
#define lmask7  ((uint)0x0000007f)
#define lmask8  ((uint)0x000000ff)
#define lmask9  ((uint)0x000001ff)
#define lmask10 ((uint)0x000003ff)
#define lmask11 ((uint)0x000007ff)
#define lmask12 ((uint)0x00000fff)
#define lmask13 ((uint)0x00001fff)
#define lmask14 ((uint)0x00003fff)
#define lmask15 ((uint)0x00007fff)
#define lmask16 ((uint)0x0000ffff)
#define lmask17 ((uint)0x0001ffff)
#define lmask18 ((uint)0x0003ffff)
#define lmask19 ((uint)0x0007ffff)
#define lmask20 ((uint)0x000fffff)
#define lmask21 ((uint)0x001fffff)
#define lmask22 ((uint)0x003fffff)
#define lmask23 ((uint)0x007fffff)
#define lmask24 ((uint)0x00ffffff)
#define lmask25 ((uint)0x01ffffff)
#define lmask26 ((uint)0x03ffffff)
#define lmask27 ((uint)0x07ffffff)
#define lmask28 ((uint)0x0fffffff)
#define lmask29 ((uint)0x1fffffff)
#define lmask30 ((uint)0x3fffffff)
#define lmask31 ((uint)0x7fffffff)
#define lmask32 ((uint)0xffffffff)

#define lmask33 ((uintl)0x00000001ffffffffull)
#define lmask34 ((uintl)0x00000003ffffffffull)
#define lmask35 ((uintl)0x00000007ffffffffull)
#define lmask36 ((uintl)0x0000000fffffffffull)
#define lmask37 ((uintl)0x0000001fffffffffull)
#define lmask38 ((uintl)0x0000003fffffffffull)
#define lmask39 ((uintl)0x0000007fffffffffull)
#define lmask40 ((uintl)0x000000ffffffffffull)
#define lmask41 ((uintl)0x000001ffffffffffull)
#define lmask42 ((uintl)0x000003ffffffffffull)
#define lmask43 ((uintl)0x000007ffffffffffull)
#define lmask44 ((uintl)0x00000fffffffffffull)
#define lmask45 ((uintl)0x00001fffffffffffull)
#define lmask46 ((uintl)0x00003fffffffffffull)
#define lmask47 ((uintl)0x00007fffffffffffull)
#define lmask48 ((uintl)0x0000ffffffffffffull)
#define lmask49 ((uintl)0x0001ffffffffffffull)
#define lmask50 ((uintl)0x0003ffffffffffffull)
#define lmask51 ((uintl)0x0007ffffffffffffull)
#define lmask52 ((uintl)0x000fffffffffffffull)
#define lmask53 ((uintl)0x001fffffffffffffull)
#define lmask54 ((uintl)0x003fffffffffffffull)
#define lmask55 ((uintl)0x007fffffffffffffull)
#define lmask56 ((uintl)0x00ffffffffffffffull)
#define lmask57 ((uintl)0x01ffffffffffffffull)
#define lmask58 ((uintl)0x03ffffffffffffffull)
#define lmask59 ((uintl)0x07ffffffffffffffull)
#define lmask60 ((uintl)0x0fffffffffffffffull)
#define lmask61 ((uintl)0x1fffffffffffffffull)
#define lmask62 ((uintl)0x3fffffffffffffffull)
#define lmask63 ((uintl)0x7fffffffffffffffull)
#define lmask64 ((uintl)0xffffffffffffffffull)

#define bit1  ((uint)1 << 0)
#define bit2  ((uint)1 << 1)
#define bit3  ((uint)1 << 2)
#define bit4  ((uint)1 << 3)
#define bit5  ((uint)1 << 4)
#define bit6  ((uint)1 << 5)
#define bit7  ((uint)1 << 6)
#define bit8  ((uint)1 << 7)
#define bit9  ((uint)1 << 8)
#define bit10 ((uint)1 << 9)
#define bit11 ((uint)1 << 10)
#define bit12 ((uint)1 << 11)
#define bit13 ((uint)1 << 12)
#define bit14 ((uint)1 << 13)
#define bit15 ((uint)1 << 14)
#define bit16 ((uint)1 << 15)
#define bit17 ((uint)1 << 16)
#define bit18 ((uint)1 << 17)
#define bit19 ((uint)1 << 18)
#define bit20 ((uint)1 << 19)
#define bit21 ((uint)1 << 20)
#define bit22 ((uint)1 << 21)
#define bit23 ((uint)1 << 22)
#define bit24 ((uint)1 << 23)
#define bit25 ((uint)1 << 24)
#define bit26 ((uint)1 << 25)
#define bit27 ((uint)1 << 26)
#define bit28 ((uint)1 << 27)
#define bit29 ((uint)1 << 28)
#define bit30 ((uint)1 << 29)
#define bit31 ((uint)1 << 30)
#define bit32 ((uint)1 << 31)

#define bit33 ((uintl)1ull << 32)
#define bit34 ((uintl)1ull << 33)
#define bit35 ((uintl)1ull << 34)
#define bit36 ((uintl)1ull << 35)
#define bit37 ((uintl)1ull << 36)
#define bit38 ((uintl)1ull << 37)
#define bit39 ((uintl)1ull << 38)
#define bit40 ((uintl)1ull << 39)
#define bit41 ((uintl)1ull << 40)
#define bit42 ((uintl)1ull << 41)
#define bit43 ((uintl)1ull << 42)
#define bit44 ((uintl)1ull << 43)
#define bit45 ((uintl)1ull << 44)
#define bit46 ((uintl)1ull << 45)
#define bit47 ((uintl)1ull << 46)
#define bit48 ((uintl)1ull << 47)
#define bit49 ((uintl)1ull << 48)
#define bit50 ((uintl)1ull << 49)
#define bit51 ((uintl)1ull << 50)
#define bit52 ((uintl)1ull << 51)
#define bit53 ((uintl)1ull << 52)
#define bit54 ((uintl)1ull << 53)
#define bit55 ((uintl)1ull << 54)
#define bit56 ((uintl)1ull << 55)
#define bit57 ((uintl)1ull << 56)
#define bit58 ((uintl)1ull << 57)
#define bit59 ((uintl)1ull << 58)
#define bit60 ((uintl)1ull << 59)
#define bit61 ((uintl)1ull << 60)
#define bit62 ((uintl)1ull << 61)
#define bit63 ((uintl)1ull << 62)
#define bit64 ((uintl)1ull << 63)

/*****************************************************************************/

typedef jmp_buf jump_point;

extern jump_point default_failure_jump_point;

#define set_jump_point(...) setjmp(__VA_ARGS__)
#define jump(...)           longjmp(__VA_ARGS__)

/*****************************************************************************/

typedef va_list vargs;

#define get_vargs(...) va_start(__VA_ARGS__)
#define end_vargs(...) va_end(__VA_ARGS__)

/*****************************************************************************/

inline uint get_maximum(uint a, uint b)
{
  return a >= b ? a : b;
}

/*****************************************************************************/

typedef wchar_t  wchar;
typedef char     utf8;
typedef wchar    utf16;
typedef char32_t utf32;

uint get_string_size(const char *string);

sintl compare_string(const utf8 *left, const utf8 *right);

sintl compare_sized_string(const utf8 *left, const utf8 *right, uint size);

#define compare_literal_string(left, right) compare_sized_string(left, right, sizeof(left) - 1)

byte decode_utf8(utf32 *rune, const utf8 string[4]);

byte encode_utf8(utf8 string[4], utf32 rune);

/*****************************************************************************/

typedef uintptr_t address;
typedef long double universal_alignment_type;

#define universal_alignment ((uint)alignof(universal_alignment_type))
#define default_alignment   ((uint)alignof(void *))

inline uint get_backward_alignment(address x, uint a)
{
  return a ? x & (a - 1) : 0;
}

inline uint get_forward_alignment(address x, uint a)
{
  uint m = get_backward_alignment(x, a);
  return m ? a - m : 0;
}

inline address align_forwards(address x, uint a)
{
  return x + get_forward_alignment(x, a);
}

/*****************************************************************************/

inline void copy_memory(void *destination, const void *source, uint size)
{
  memcpy(destination, source, size);
}

inline void fill_memory(byte value, void *destination, uint size)
{
  memset(destination, value, size);
}

inline void move_memory(void *destination, const void *source, uint size)
{
  memmove(destination, source, size);
}

/*****************************************************************************/

#define kibibyte ((uint)1024)
#define mebibyte ((uint)kibibyte * kibibyte)
#define memory_page_size ((uint)4 * kibibyte)

extern uint global_memory_usage;

void *allocate_memory(uint size);

void *reallocate_memory(uint new_size, void *memory, uint size);

void release_memory(void *memory, uint size);

/*****************************************************************************/

typedef struct linear_allocator_chunk linear_allocator_chunk;
struct linear_allocator_chunk
{
  uint size;
  uint mass;
  byte *memory;
  linear_allocator_chunk *prior;
  alignas(universal_alignment) byte tailing_memory[];
};

typedef struct
{
  linear_allocator_chunk *chunk;
  uint minimum_chunk_size;
  bit enabled_nullify : 1;
} linear_allocator;

#define default_linear_allocator_minimum_chunk_size ((uint)4096)

void *allocate_from_linear_allocator(uint size, uint alignment, linear_allocator *state);

void release_from_linear_allocator(uint size, uint alignment, linear_allocator *state);

/*****************************************************************************/

typedef struct chunk_allocator_block chunk_allocator_block;
struct chunk_allocator_block
{
  uint count;
  uint *bits;
  byte *chunks; 
  chunk_allocator_block *prior;
  alignas(alignof(uintl)) byte tailing_memory[];
};

typedef struct
{
  uint minimum_block_chunks_count;
  uint chunk_size;
  chunk_allocator_block *block;
  bit enabled_nullify : 1;
} chunk_allocator;

#define default_chunk_allocator_minimum_block_chunks_count ((uint)64)
#define default_chunk_allocator_chunk_size                 ((uint)64)

void *allocate_from_chunk_allocator(uint size, chunk_allocator *state);

void release_from_chunk_allocator(uint index, uint size, chunk_allocator *state);

/*****************************************************************************/

typedef enum
{
  linear_allocator_type,
  chunk_allocator_type,
} allocator_type;

typedef void *allocate_procedure(uint size, uint alignment, void *state);

typedef void release_procedure(uint size, uint alignment, void *state);

typedef struct
{
  allocator_type type;
  void *state;
  allocate_procedure *allocate;
  release_procedure *release;
} allocator;

/*****************************************************************************/

typedef struct
{
  allocator *allocator;

  jump_point *failure_jump_point;
} context_data;

extern thread_local context_data context;

void *push(uint size, uint alignment);

#define push_type(type, count) (type *)push(count * sizeof(type), alignof(type))

void pop(uint size, uint alignment);

#define pop_type(type, count) pop(count * sizeof(type), alignof(type))

/*****************************************************************************/

#if defined(ON_PLATFORM_WIN32)
typedef HANDLE handle;
#endif

#define maximum_path_size ((uint)MAX_PATH)

handle open_file(const char *file_path);

uintl get_file_size(handle file_handle);

uint read_from_file(void *buffer, uint buffer_size, handle file_handle);

void close_file(handle file_handle);
