#include "proglosa.c"

void test_index_bit_range(void);

int main(void)
{
  test_index_bit_range();
}

void test_index_bit_range(void)
{
  {
    uint bytes[] = { 0b00000000000000000000000011100011 };
    sintl index = index_bit_range(3, false, bytes, 1);
    printf("index: %lli\n", index);
    assert(index == 5);
  }

  {
    uint bytes[] = { 0b00000000000000000000000000011100, 0 };
    sintl index = index_bit_range(3, false, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == 2);
  }

  {
    uint bytes[] = { 0b11111111111111111111111111111111, 0 };
    sintl index = index_bit_range(32, false, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == 0);
  }

  {
    uint bytes[] = { 0b11111111111111111111111111000000, 0b11111111111111111111111111111111 };
    sintl index = index_bit_range(58, false, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == 6);
  }

  {
    uint bytes[] = { 0b00011100 };
    sintl index = index_bit_range(3, false, bytes, 1);
    printf("index: %lli\n", index);
    assert(index == 2);
  }

  {
    uint bytes[] = { 0b10000000000000000000000000000000, 0b00000000000000000000000000000011 };
    sintl index = index_bit_range(3, false, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == 31);
  }
  
  {
    uint bytes[] = { 0b00000000000000000000000000000000, 0b00000000000000000000000000000111 };
    sintl index = index_bit_range(3, false, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == 32);
  }

  {
    uint bytes[] = { 0b00000000000000000000000000000000, 0b00000000000000000000000000000000 };
    sintl index = index_bit_range(3, false, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == -1);
  }

  {
    uint bytes[] = { 0 };
    sintl index = index_bit_range(3, false, bytes, 1);
    printf("index: %lli\n", index);
    assert(index == -1);
  }

  {
    uint bytes[] = { 0b11111111111111111111111111100011, 0 };
    sintl index = index_bit_range(3, true, bytes, 2);
    printf("index: %lli\n", index);
    assert(index == 2);
  }
}
