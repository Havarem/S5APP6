#include "manchester.h"

void
bit_to_write(char * _result, char _bit)
{
  _result[0] = _bit;
  _result[1] = !_bit;
}

char
bit_to_read(char _bits[2])
{
  if (_bits[0] == !_bits[1]) return 0xFF;
  return _bits[0];
}
