#pragma once

#if defined(_WIN32)
  #define ON_PLATFORM_WIN32 1
#endif

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
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

#define alignof(...) _Alignof(__VA_ARGS__)
#define alignas(...) _Alignas(__VA_ARGS__)

/*****************************************************************************/

#define note(...) printf("[note] " __VA_ARGS__)
#define warn(...) printf("[warn] " __VA_ARGS__)
#define err(...)  printf("[err] " __VA_ARGS__)

/*****************************************************************************/

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef u16 uints;
typedef u32 uint;
typedef u64 uintl;

typedef s16 sints;
typedef s32 sint;
typedef s64 sintl;

typedef u8 byte;
typedef u8 bit;

static const uint uint_maximum_value = ~(uint)0;

static const uint lmask1  = 0x00000001;
static const uint lmask2  = 0x00000003;
static const uint lmask3  = 0x00000007;
static const uint lmask4  = 0x0000000f;
static const uint lmask5  = 0x0000001f;
static const uint lmask6  = 0x0000003f;
static const uint lmask7  = 0x0000007f;
static const uint lmask8  = 0x000000ff;
static const uint lmask9  = 0x000001ff;
static const uint lmask10 = 0x000003ff;
static const uint lmask11 = 0x000007ff;
static const uint lmask12 = 0x00000fff;
static const uint lmask13 = 0x00001fff;
static const uint lmask14 = 0x00003fff;
static const uint lmask15 = 0x00007fff;
static const uint lmask16 = 0x0000ffff;
static const uint lmask17 = 0x0001ffff;
static const uint lmask18 = 0x0003ffff;
static const uint lmask19 = 0x0007ffff;
static const uint lmask20 = 0x000fffff;
static const uint lmask21 = 0x001fffff;
static const uint lmask22 = 0x003fffff;
static const uint lmask23 = 0x007fffff;
static const uint lmask24 = 0x00ffffff;
static const uint lmask25 = 0x01ffffff;
static const uint lmask26 = 0x03ffffff;
static const uint lmask27 = 0x07ffffff;
static const uint lmask28 = 0x0fffffff;
static const uint lmask29 = 0x1fffffff;
static const uint lmask30 = 0x3fffffff;
static const uint lmask31 = 0x7fffffff;
static const uint lmask32 = 0xffffffff;

static const uintl lmask33 = 0x00000001ffffffffull;
static const uintl lmask34 = 0x00000003ffffffffull;
static const uintl lmask35 = 0x00000007ffffffffull;
static const uintl lmask36 = 0x0000000fffffffffull;
static const uintl lmask37 = 0x0000001fffffffffull;
static const uintl lmask38 = 0x0000003fffffffffull;
static const uintl lmask39 = 0x0000007fffffffffull;
static const uintl lmask40 = 0x000000ffffffffffull;
static const uintl lmask41 = 0x000001ffffffffffull;
static const uintl lmask42 = 0x000003ffffffffffull;
static const uintl lmask43 = 0x000007ffffffffffull;
static const uintl lmask44 = 0x00000fffffffffffull;
static const uintl lmask45 = 0x00001fffffffffffull;
static const uintl lmask46 = 0x00003fffffffffffull;
static const uintl lmask47 = 0x00007fffffffffffull;
static const uintl lmask48 = 0x0000ffffffffffffull;
static const uintl lmask49 = 0x0001ffffffffffffull;
static const uintl lmask50 = 0x0003ffffffffffffull;
static const uintl lmask51 = 0x0007ffffffffffffull;
static const uintl lmask52 = 0x000fffffffffffffull;
static const uintl lmask53 = 0x001fffffffffffffull;
static const uintl lmask54 = 0x003fffffffffffffull;
static const uintl lmask55 = 0x007fffffffffffffull;
static const uintl lmask56 = 0x00ffffffffffffffull;
static const uintl lmask57 = 0x01ffffffffffffffull;
static const uintl lmask58 = 0x03ffffffffffffffull;
static const uintl lmask59 = 0x07ffffffffffffffull;
static const uintl lmask60 = 0x0fffffffffffffffull;
static const uintl lmask61 = 0x1fffffffffffffffull;
static const uintl lmask62 = 0x3fffffffffffffffull;
static const uintl lmask63 = 0x7fffffffffffffffull;
static const uintl lmask64 = 0xffffffffffffffffull;

static const uint bit1  = 1 << 0;
static const uint bit2  = 1 << 1;
static const uint bit3  = 1 << 2;
static const uint bit4  = 1 << 3;
static const uint bit5  = 1 << 4;
static const uint bit6  = 1 << 5;
static const uint bit7  = 1 << 6;
static const uint bit8  = 1 << 7;
static const uint bit9  = 1 << 8;
static const uint bit10 = 1 << 9;
static const uint bit11 = 1 << 10;
static const uint bit12 = 1 << 11;
static const uint bit13 = 1 << 12;
static const uint bit14 = 1 << 13;
static const uint bit15 = 1 << 14;
static const uint bit16 = 1 << 15;
static const uint bit17 = 1 << 16;
static const uint bit18 = 1 << 17;
static const uint bit19 = 1 << 18;
static const uint bit20 = 1 << 19;
static const uint bit21 = 1 << 20;
static const uint bit22 = 1 << 21;
static const uint bit23 = 1 << 22;
static const uint bit24 = 1 << 23;
static const uint bit25 = 1 << 24;
static const uint bit26 = 1 << 25;
static const uint bit27 = 1 << 26;
static const uint bit28 = 1 << 27;
static const uint bit29 = 1 << 28;
static const uint bit30 = 1 << 29;
static const uint bit31 = 1 << 30;
static const uint bit32 = 1 << 31;

static const uintl bit33 = 1ull << 32;
static const uintl bit34 = 1ull << 33;
static const uintl bit35 = 1ull << 34;
static const uintl bit36 = 1ull << 35;
static const uintl bit37 = 1ull << 36;
static const uintl bit38 = 1ull << 37;
static const uintl bit39 = 1ull << 38;
static const uintl bit40 = 1ull << 39;
static const uintl bit41 = 1ull << 40;
static const uintl bit42 = 1ull << 41;
static const uintl bit43 = 1ull << 42;
static const uintl bit44 = 1ull << 43;
static const uintl bit45 = 1ull << 44;
static const uintl bit46 = 1ull << 45;
static const uintl bit47 = 1ull << 46;
static const uintl bit48 = 1ull << 47;
static const uintl bit49 = 1ull << 48;
static const uintl bit50 = 1ull << 49;
static const uintl bit51 = 1ull << 50;
static const uintl bit52 = 1ull << 51;
static const uintl bit53 = 1ull << 52;
static const uintl bit54 = 1ull << 53;
static const uintl bit55 = 1ull << 54;
static const uintl bit56 = 1ull << 55;
static const uintl bit57 = 1ull << 56;
static const uintl bit58 = 1ull << 57;
static const uintl bit59 = 1ull << 58;
static const uintl bit60 = 1ull << 59;
static const uintl bit61 = 1ull << 60;
static const uintl bit62 = 1ull << 61;
static const uintl bit63 = 1ull << 62;
static const uintl bit64 = 1ull << 63;

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

enum
{
  universal_alignment = alignof(universal_alignment_type),
  default_alignment   = alignof(void *),
};

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

inline void fill_memory(void *destination, uint size, byte value)
{
  memset(destination, value, size);
}

inline void move_memory(void *destination, const void *source, uint size)
{
  memmove(destination, source, size);
}

/*****************************************************************************/

enum
{
  kibibyte = 1024,
  mebibyte = kibibyte * kibibyte,

  memory_page_size = 4 * kibibyte,
};

extern uint global_memory_usage;

void *allocate_memory(uint size);

void *reallocate_memory(uint new_size, void *memory, uint size);

void release_memory(void *memory, uint size);

/*****************************************************************************/

typedef struct chunk chunk;
struct chunk
{
  uint size;
  uint mass;
  byte *memory;
  chunk *prior;
  alignas(universal_alignment) byte tailing_memory[];
};

typedef struct
{
  chunk *chunk;
  uint minimum_chunk_size;
  bit enabled_nullify : 1;
} linear_allocator;

enum { default_linear_allocator_minimum_chunk_size = 4096 };

void *push_into_linear_allocator(uint size, uint alignment, linear_allocator *state);

void pop_from_linear_allocator(uint size, uint alignment, linear_allocator *state);

/*****************************************************************************/

typedef enum
{
  linear_allocator_type = 0x01,
} allocator_type;

typedef void *push_procedure(uint size, uint alignment, void *state);
typedef void pop_procedure(uint size, uint alignment, void *state);

typedef struct
{
  allocator_type type;
  void *state;
  push_procedure *push;
  pop_procedure *pop;
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

enum { maximum_path_size = MAX_PATH };

handle open_file(const char *file_path);

uintl get_file_size(handle file_handle);

uint read_from_file(void *buffer, uint buffer_size, handle file_handle);

void close_file(handle file_handle);
