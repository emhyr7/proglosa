#include "base.h"

/*****************************************************************************/

uintb clz(uintl value)
{
  unsigned long index;
  if (!_BitScanReverse64(&index, value))
    return sizeof(value) * byte_width;
  return sizeof(value) * byte_width - index - 1;
}

uintb ctz(uintl value)
{
  unsigned long index;
  if (!_BitScanForward64(&index, value))
    return sizeof(value) * byte_width;
  return index;
}

/*
ALGORITHM

  twice the byte granularity of bits are buffered as a bitmask that's
  iteratively compared to the input bitset by shifting the bitmask.

  the amount of shifts for the bitmask is calculated by:
  1) the amount of trailing zeros in the bitset; and
  2) the amount of leading zeros in the bitmask.
  in other words, we skip ranges of zeros, and try to skip to a leading range
  of ones in the bitmask. (we "try" since there may not be any leading range of
  ones in the bitmask, at which case the bitmask is completely skipped).

  when the total amount of shifts is clamped and reset at the the byte
  granularity, we extract the next byte from the bitset into the bitmask.
*/
sintl index_bit_range(uintb range_size, bit of_zeros, const uint *bytes, uint bytes_count)
{
  sintl range_index = -1;
  uintb byte_granularity = sizeof(*bytes) * byte_width; 

  /* i know... it's not ideal to limit the range */
  assert(range_size <= byte_granularity * 2);

  uintl mask = (1 << range_size) - 1;
  uintb shift_count = 0;
  uint byte_index = 0;
  for (;;)
  {
    if (byte_index == bytes_count) break;

    /* buffer bits as a bitmask */
    uintl buffer = (uintl)0 | (uintl)bytes[byte_index];
    if (byte_index != bytes_count - 1) buffer |= ((uintl)bytes[byte_index + 1] << byte_granularity);

    /* flip the bits because the user wants to search for a range of zeros
       instead of a range of ones */
    if (of_zeros) buffer = ~buffer;

    /* skip the trailing zeros */
    shift_count += ctz(buffer >> shift_count);

    /* try to extract the next byte */
    if (shift_count >= byte_granularity)
    {
      ++byte_index;
      shift_count = 0;
      continue;
    }

    /* skip the compare the bitmask */
    uintl shifted_mask = mask << shift_count;
    uintl masked = buffer & shifted_mask;
    if (masked == shifted_mask)
    {
      range_index = byte_index * byte_granularity + shift_count;
      break;
    }

    /* try to skip to the leading range of ones */
    uintb leading_zeros_count = clz(~masked << (byte_granularity * 2 - range_size));
    shift_count += range_size - leading_zeros_count;
  }
  return range_index;
}

sintl toggle_bit_range(uintb range_size, bit of_zero, uint *bytes, uint bytes_count)
{
  sintl index = index_bit_range(range_size, of_zero, bytes, bytes_count);
  if (index < 0) return -1;
  uint byte_granularity = sizeof(uint) * byte_width;
  uint *byte = bytes + index / byte_granularity;
  *byte ^= ((1 << range_size) - 1) << (index % byte_granularity);
  return index;
}

/*****************************************************************************/

inline uint get_maximum(uint a, uint b)
{
  return a >= b ? a : b;
}

/*****************************************************************************/

inline uint get_string_size(const char *string)
{
  return (uint)strlen(string);
}

inline sintl compare_string(const utf8 *left, const utf8 *right)
{
  return strcmp(left, right);
}

inline sintl compare_sized_string(const utf8 *left, const utf8 *right, uint size)
{
  return strncmp(left, right, size);
}

static const byte utf8_classes[32] =
{
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0,
  2, 2, 2, 2,
  3, 3,
  4,
  5,
};

byte decode_utf8(utf32 *rune, const utf8 string[4])
{
  byte bytes[4] = { string[0] };
  byte utf8_class = utf8_classes[bytes[0] >> 3];
  switch (utf8_class)
  {
  case 1:
    *rune = bytes[0];
    break;
  case 2:
    {
      bytes[1] = string[1];
      if (!utf8_classes[bytes[1] >> 3])
      {
        *rune = ((bytes[0] & lmask5) << 6)
              | ((bytes[1] & lmask6) << 0);
      }
      break;
    }
  case 3:
    {
      bytes[1] = string[1];
      bytes[2] = string[2];
      if (!utf8_classes[bytes[1] >> 3] &&
          !utf8_classes[bytes[2] >> 3])
      {
        *rune = ((bytes[0] & lmask4) << 12)
              | ((bytes[1] & lmask6) <<  6)
              | ((bytes[2] & lmask6) <<  0);
      }
      break;
    }
  case 4:
    {
      bytes[1] = string[1];
      bytes[2] = string[2];
      bytes[3] = string[3];
      if (!utf8_classes[bytes[1] >> 3] &&
          !utf8_classes[bytes[2] >> 3] &&
          !utf8_classes[bytes[3] >> 3])
      {
        *rune = ((bytes[0] & lmask3) << 18)
              | ((bytes[1] & lmask6) << 12)
              | ((bytes[2] & lmask6) <<  6)
              | ((bytes[3] & lmask6) <<  0);
      }
      break;
    }
  default:
    utf8_class = 0;
    break;
  }
  return utf8_class;
}

byte encode_utf8(utf8 string[4], utf32 rune)
{
  byte increment;
  if (rune <= 0x7f)
  {
    string[0] = (utf8)rune;
    increment = 1;
  }
  else if (rune <= 0x7ff)
  {
    string[0] = (lmask2 << 6) | ((rune >> 6) & lmask5);
    string[1] = bit8 | ((rune >> 0) & lmask6);
    increment = 2;
  }
  else if (rune <= 0xffff)
  {
    string[0] = (lmask3 << 5) | ((rune >> 12) & lmask4);
    string[1] = bit8 | ((rune >> 6) & lmask6);
    string[2] = bit8 | ((rune >> 0) & lmask6);
    increment = 3;
  }
  else if (rune <= 0x10ffff)
  {
    string[0] = (lmask4 << 4) | ((rune >> 18) & lmask3);
    string[1] = bit8 | ((rune >> 12) & lmask6);
    string[2] = bit8 | ((rune >>  6) & lmask6);
    string[3] = bit8 | ((rune >>  0) & lmask6);
    increment = 4;
  }
  else
  {
    string[0] = '?';
    increment = 1;
  }
  return increment;
}

/*****************************************************************************/

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

inline void copy(void *destination, const void *source, uint size)
{
  CopyMemory(destination, source, size);
}

inline void fill(void *destination, uint size, byte value)
{
  FillMemory(destination, size, value);
}

inline void zero(void *destination, uint size)
{
  ZeroMemory(destination, size);
}

inline void move(void *destination, const void *source, uint size)
{
  MoveMemory(destination, source, size);
}

inline void *allocate(uint size)
{
  void *memory = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (!memory)
  {
    print_failure("loser !");
    jump(*context.failure_jump_point, 1);
  }
  return memory;
}

inline void deallocate(void *memory, uint size)
{
  VirtualFree(memory, 0, MEM_RELEASE);
}

inline void *reallocate(uint size, void *old_memory, uint old_size)
{
  void *memory = allocate(size);
  copy(memory, old_memory, old_size);
  deallocate(old_memory, old_size);
  return memory;
}

void *push(uint size, uint alignment, allocator *allocator)
{
  if (!allocator->active_buffer) allocator->active_buffer = allocator->first_buffer;

  uint forward_alignment;
  bit would_overflow = 1;
  if (allocator->active_buffer)
  {
    for (;;)
    {
      forward_alignment = get_forward_alignment((address)allocator->active_buffer->memory + allocator->active_buffer->mass, alignment);
      would_overflow = allocator->active_buffer->mass + forward_alignment + size > allocator->active_buffer->size;
      if (!would_overflow) break;
      if (!allocator->active_buffer->next) break;
      allocator->active_buffer = allocator->active_buffer->next;
    }
  }

  if (would_overflow)
  {
    forward_alignment = 0;
    if (!allocator->minimum_buffer_size) allocator->minimum_buffer_size = default_allocator_minimum_buffer_size;
    uint buffer_size = get_maximum(size, allocator->minimum_buffer_size);
    uint allocation_size = sizeof(buffer) + buffer_size;
    buffer *new_buffer = allocator->allocator ? push(allocation_size, alignof(buffer), allocator->allocator) : allocate(allocation_size);
    new_buffer->prior = allocator->active_buffer;
    if (allocator->active_buffer) allocator->active_buffer->next = new_buffer;
    new_buffer->mass = 0;
    new_buffer->size = buffer_size;
    new_buffer->memory = new_buffer->tailing_memory;
    new_buffer->next = 0;
    allocator->active_buffer = new_buffer;
    if (!allocator->first_buffer) allocator->first_buffer = new_buffer;
  }

  allocator->active_buffer->mass += forward_alignment;
  void *memory = allocator->active_buffer->memory + allocator->active_buffer->mass;
  allocator->active_buffer->mass += size;
  fill(memory, size, 0);
  return memory;
}

void get_scratch(scratch *scratch, allocator *allocator)
{
  scratch->allocator = allocator;
  scratch->buffer = allocator->active_buffer;
  scratch->mass = allocator->active_buffer ? allocator->active_buffer->mass : 0;
}

void end_scratch(scratch *scratch)
{
  buffer *current_buffer = scratch->allocator->active_buffer;
  for (
    buffer *prior_buffer;
    current_buffer && current_buffer != scratch->buffer;
    current_buffer = prior_buffer)
  {
    prior_buffer = current_buffer->prior;
    if (!scratch->allocator->allocator) deallocate(current_buffer, sizeof(buffer) + current_buffer->size);
    else current_buffer->mass = 0;
  }
  ASSERT(current_buffer == scratch->buffer);
  if (current_buffer) current_buffer->mass = scratch->mass;
  scratch->allocator->active_buffer = current_buffer;
}

/*****************************************************************************/

/* this is initialized at runtime in `initialize_context` */
thread_local context_data context;

static void initialize_context(void)
{
  context.allocator = &context.default_allocator;
  context.failure_jump_point = &context.default_failure_jump_point;
}

/*****************************************************************************/

handle open_file(const char *file_path)
{
  HANDLE file_handle = CreateFileA(file_path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if (file_handle == INVALID_HANDLE_VALUE)
  {
    print_failure("Failed to open file.\n");
    jump(*context.failure_jump_point, 1);
  }
  return (handle)file_handle;
}

uintl get_file_size(handle file_handle)
{
  LARGE_INTEGER large_integer;
  if (!GetFileSizeEx(file_handle, &large_integer))
  {
    print_failure("Failed to get file size.\n");
    jump(*context.failure_jump_point, 1);
  }
  return large_integer.QuadPart;
}

uint read_from_file(void *buffer, uint buffer_size, handle file_handle)
{
  DWORD read_size;
  if (!ReadFile(file_handle, buffer, buffer_size, &read_size, 0))
  {
    print_failure("Failed to read file.\n");
    jump(*context.failure_jump_point, 1);
  }
  return read_size;
}

void close_file(handle file_handle)
{
  CloseHandle(file_handle);
}

/*****************************************************************************/

uintl clock_frequency;

thread_local uintl clock_beginning_time;

inline uintl get_time(void)
{
  LARGE_INTEGER time;
  QueryPerformanceCounter(&time);
  return time.QuadPart;
}

inline void begin_clock(void)
{
  clock_beginning_time = get_time();
}

inline float64 end_clock(void)
{
  return (float64)(get_time() - clock_beginning_time) * 1000000 / clock_frequency;
}

/*****************************************************************************/

bit initialize_base(void)
{
  {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = frequency.QuadPart;
  }

  if (set_jump_point(context.default_failure_jump_point))
  {
    print_comment("Failed to %s.", __FUNCTION__);
    return 0;
  }
  
  initialize_context();
  return 1;
}
