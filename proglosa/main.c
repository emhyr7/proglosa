#include "proglosa.c"

void test_index_bit_range(void);

void test_chunk_allocator(void);

int main(void)
{
  printf("universal_alignment: %u\n", universal_alignment);
  /* test_index_bit_range(); */
  test_chunk_allocator();
}

void test_chunk_allocator(void)
{
  chunk_allocator state =
  {
    .minimum_block_chunks_count = default_chunk_allocator_minimum_block_chunks_count,
    .chunk_size                 = default_chunk_allocator_chunk_size,
    .block                      = 0,
    .enabled_nullify            = 1,
  };

  {
    allocate_from_chunk_allocator(3 * state.chunk_size, &state);
    release_from_chunk_allocator(0, state.chunk_size, &state);   
    allocate_from_chunk_allocator(3 * state.chunk_size, &state);
  }
}

void test_index_bit_range(void)
{
  {
    uint bytes[] = { 0b00000000000000000000000011100011 };
    sintl index = index_bit_range(3, 0, bytes, 1);
    printf("index: %lli\n", index);
    assert(index == 5);
  }

  {
    uint bytes[] = { 0b00000000000000000000000000011100, 0 };
    sintl index = index_bit_range(3, 0, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == 2);
  }

  {
    uint bytes[] = { 0b11111111111111111111111111111111, 0 };
    sintl index = index_bit_range(32, 0, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == 0);
  }

  {
    uint bytes[] = { 0b11111111111111111111111111000000, 0b11111111111111111111111111111111 };
    sintl index = index_bit_range(58, 0, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == 6);
  }

  {
    uint bytes[] = { 0b00011100 };
    sintl index = index_bit_range(3, 0, bytes, 1);
    printf("index: %lli\n", index);
    assert(index == 2);
  }

  {
    uint bytes[] = { 0b10000000000000000000000000000000, 0b00000000000000000000000000000011 };
    sintl index = index_bit_range(3, 0, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == 31);
  }
  
  {
    uint bytes[] = { 0b00000000000000000000000000000000, 0b00000000000000000000000000000111 };
    sintl index = index_bit_range(3, 0, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == 32);
  }

  {
    uint bytes[] = { 0b00000000000000000000000000000000, 0b00000000000000000000000000000000 };
    sintl index = index_bit_range(3, 0, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == -1);
  }

  {
    uint bytes[] = { 0 };
    sintl index = index_bit_range(3, 0, bytes, 1);
    printf("index: %lli\n", index);
    assert(index == -1);
  }

  {
    uint bytes[] = { 0b11111111111111111111111111100011, 0 };
    sintl index = index_bit_range(3, 1, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == 2);
  }
}
