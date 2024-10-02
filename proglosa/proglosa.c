#include "proglosa.h"
#include "proglosa_parsing.h"

static void initialize(void);

int main(int arguments_count, char **arguments)
{
  initialize();

  if (arguments_count <= 1)
  {
    printf("error: A source path wasn't given.");
    return -1;
  }

  parser parser;
  const utf8 *source_path = arguments[1];
  load_into_parser(source_path, &parser);
  for (;;)
  {
    token_type type = tokenize(&parser);
    const utf8 *representation = token_type_representations[type];
    report_token_comment(&parser.token, parser.source, parser.source_path, representation);
    if (type == etx_token_type)
      break;
  }
  return 0;
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

void *push_into_linear_allocator(uint size, uint alignment, linear_allocator *state)
{
  uint forward_alignment = state->chunk ? get_forward_alignment((address)state->chunk->memory + state->chunk->mass, alignment) : 0; 
  if (!state->chunk || state->chunk->mass + forward_alignment + size > state->chunk->size)
  {
    state->minimum_chunk_size = get_maximum(default_linear_allocator_minimum_chunk_size, state->minimum_chunk_size);
    uint new_chunk_size = get_maximum(size, state->minimum_chunk_size);
    chunk *new_chunk = (chunk *)allocate_memory(sizeof(chunk) + new_chunk_size);
    new_chunk->size = new_chunk_size;
    new_chunk->mass = 0;
    new_chunk->memory = new_chunk->tailing_memory;
    new_chunk->prior = state->chunk;
    state->chunk = new_chunk;
  }
  forward_alignment = get_forward_alignment((address)state->chunk->memory + state->chunk->mass, alignment);
  void *memory = state->chunk->memory + state->chunk->mass + forward_alignment;
  if (state->enabled_nullify) fill_memory(memory, size, 0);
  state->chunk->mass += forward_alignment + size;
  return memory;
}

void pop_from_linear_allocator(uint size, uint alignment, linear_allocator *state)
{
  while (state->chunk && size)
  {
    uint mass = state->chunk->mass;
    size += get_backward_alignment((address)state->chunk->memory + mass, alignment);
    bool releasable_chunk = (sintl)mass - size <= 0;
    state->chunk->mass -= size;
    if (releasable_chunk)
    {
      chunk *releasable_chunk = state->chunk;
      state->chunk = state->chunk->prior;
      release_memory(releasable_chunk, releasable_chunk->size);
      size -= mass;
    }
    else size = 0;
  }
}

/*****************************************************************************/

thread_local linear_allocator default_allocator_state;

thread_local allocator default_allocator =
{
  .type  = linear_allocator_type,
  .state = 0, /* initialized at runtime */
  .push  = (push_procedure *)&push_into_linear_allocator,
  .pop   = (pop_procedure *)&pop_from_linear_allocator,
};

/*****************************************************************************/

thread_local context ctx =
{
  .allocator = 0, /* initialized at runtime */
};

/*****************************************************************************/

handle open_file(const char *file_path)
{
  OFSTRUCT of;
  HFILE file_handle = OpenFile(file_path, &of, OF_READWRITE);
  assert(file_handle != HFILE_ERROR);
  return (handle)file_handle;
}

uintl get_file_size(handle file_handle)
{
  LARGE_INTEGER large_integer;
  assert(GetFileSizeEx(file_handle, &large_integer));
  return large_integer.QuadPart;
}

uint read_from_file(void *buffer, uint buffer_size, handle file_handle)
{
  DWORD read_size;
  assert(ReadFile(file_handle, buffer, buffer_size, &read_size, 0));
  return read_size;
}

void close_file(handle file_handle)
{
  CloseHandle(file_handle);
}

/*****************************************************************************/

static void initialize(void)
{
  default_allocator.state = &default_allocator_state;
  ctx.allocator = &default_allocator;
}
