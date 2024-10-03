#include "proglosa_commons.h"

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

/* this is half-optimized just 'cause
      - Emhyr 3.10.2024 */
sintl index_bit_range(uintb range_size, bit of_zeros, const uint *bytes, uint bytes_count)
{
  sintl range_index = -1;
  uintb byte_granularity = sizeof(*bytes) * byte_width; 
  assert(range_size <= byte_granularity * 2);

  uintl mask = (1 << range_size) - 1;
  uintb shift_count = 0;
  uint byte_index = 0;
  for (;;)
  {
    /* FUCK FUCKING FUCKS FUCK BITWISE SHIT FUCK FUCKS FUCK BITS */
    if (byte_index == bytes_count) break;

    uintl buffer = (uintl)0 | (uintl)bytes[byte_index];
    if (byte_index != bytes_count - 1) buffer |= ((uintl)bytes[byte_index + 1] << byte_granularity);
    if (of_zeros) buffer = ~buffer;

    shift_count += ctz(buffer >> shift_count);
    if (shift_count >= byte_granularity)
    {
      ++byte_index;
      shift_count = 0;
      continue;
    }

    uintl shifted_mask = mask << shift_count;
    uintl masked = buffer & shifted_mask;
    if (masked == shifted_mask)
    {
      range_index = byte_index * byte_granularity + shift_count;
      break;
    }

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

uint get_string_size(const char *string)
{
  return (uint)strlen(string);
}

sintl compare_string(const utf8 *left, const utf8 *right)
{
  return strcmp(left, right);
}

sintl compare_sized_string(const utf8 *left, const utf8 *right, uint size)
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

uint global_memory_usage = 0;

void *allocate_memory(uint size)
{
  void *memory = malloc(size);
  global_memory_usage += size;
  return memory;
}

void *reallocate_memory(uint new_size, void *memory, uint size)
{
  memory = realloc(memory, new_size);
  global_memory_usage -= size;
  global_memory_usage += new_size;
  return memory;
}

void release_memory(void *memory, uint size)
{
  free(memory);
  global_memory_usage -= size;
}

/*****************************************************************************/

void *allocate_from_linear_allocator(uint size, uint alignment, linear_allocator *state)
{
  uint forward_alignment = state->chunk ? get_forward_alignment((address)state->chunk->memory + state->chunk->mass, alignment) : 0; 
  if (!state->chunk || state->chunk->mass + forward_alignment + size > state->chunk->size)
  {
    state->minimum_chunk_size = get_maximum(default_linear_allocator_minimum_chunk_size, state->minimum_chunk_size);
    uint new_chunk_size = get_maximum(size, state->minimum_chunk_size);
    linear_allocator_chunk *new_chunk = (linear_allocator_chunk *)allocate_memory(sizeof(*new_chunk) + new_chunk_size);
    new_chunk->size = new_chunk_size;
    new_chunk->mass = 0;
    new_chunk->memory = new_chunk->tailing_memory;
    new_chunk->prior = state->chunk;
    state->chunk = new_chunk;
  }
  forward_alignment = get_forward_alignment((address)state->chunk->memory + state->chunk->mass, alignment);
  void *memory = state->chunk->memory + state->chunk->mass + forward_alignment;
  if (state->enabled_nullify) fill_memory(0, memory, size);
  state->chunk->mass += forward_alignment + size;
  return memory;
}

void release_from_linear_allocator(uint size, uint alignment, linear_allocator *state)
{
  while (state->chunk && size)
  {
    uint mass = state->chunk->mass;
    size += get_backward_alignment((address)state->chunk->memory + mass, alignment);
    bit releasable_chunk = (sintl)mass - size <= 0;
    state->chunk->mass -= size; /* this can over subtract which is okay because we'll release it at that circumstance */ 
    if (releasable_chunk)
    {
      linear_allocator_chunk *releasable_chunk = state->chunk;
      state->chunk = state->chunk->prior;
      release_memory(releasable_chunk, releasable_chunk->size);
      size -= mass;
    }
    else size = 0;
  }
}

/*****************************************************************************/

void *allocate_from_chunk_allocator(uint size, chunk_allocator *state)
{
  uint byte_granularity = sizeof(uint) * byte_width;
  uint allocated_chunks_count = size / state->chunk_size + (size % state->chunk_size > 0);
  if (!state->block)
  {
  allocate_new_block:
    uint new_block_count = state->minimum_block_chunks_count + allocated_chunks_count;
    uint new_block_bits_size = align_forwards(new_block_count / byte_granularity, universal_alignment);
    uint new_block_size = sizeof(chunk_allocator_block) + new_block_bits_size + state->chunk_size * new_block_count;
    chunk_allocator_block *new_block = (chunk_allocator_block *)allocate_memory(new_block_size);
    fill_memory(0, new_block, new_block_size);
    new_block->count = new_block_count;
    new_block->bits = (uint *)new_block->tailing_memory;
    new_block->chunks = new_block->tailing_memory + new_block_bits_size;
    new_block->prior = state->block;
    state->block = new_block;
  }
  sintl allocated_chunks_index = toggle_bit_range(allocated_chunks_count, 1, state->block->bits, state->block->count);
  if (allocated_chunks_index < 0) goto allocate_new_block;
  void *chunk = state->block->chunks + allocated_chunks_index * state->chunk_size;
  if (state->enabled_nullify) fill_memory(0, chunk, state->chunk_size);
  return chunk;
}

void release_from_chunk_allocator(uint index, uint size, chunk_allocator *state)
{
  if (state->block)
  {
    size /= state->chunk_size + (size % state->chunk_size > 0);
    uint byte_granularity = sizeof(uint) * byte_width;
    uint *byte = state->block->bits + index / byte_granularity;
    *byte ^= ((1 << size) - 1) << (index % byte_granularity);   
  }
}

/*****************************************************************************/

thread_local linear_allocator default_allocator_state;

thread_local allocator default_allocator =
{
  .type     = linear_allocator_type,
  .state    = 0, /* initialized at runtime */
  .allocate = (allocate_procedure *)&allocate_from_linear_allocator,
  .release  = (release_procedure *)&release_from_linear_allocator,
};

/*****************************************************************************/

jump_point default_failure_jump_point;

thread_local context_data context =
{
  .allocator = 0, /* initialized at runtime */
  .failure_jump_point = &default_failure_jump_point,
};

void *push(uint size, uint alignment)
{
  allocator *allocator = context.allocator;
  return allocator->allocate(size, alignment, allocator->state);
}

void pop(uint size, uint alignment)
{
  allocator *allocator = context.allocator;
  allocator->release(size, alignment, allocator->state);
}

/*****************************************************************************/

handle open_file(const char *file_path)
{
  OFSTRUCT of;
  HFILE file_handle = OpenFile(file_path, &of, OF_READWRITE);
  if (file_handle == HFILE_ERROR)
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
