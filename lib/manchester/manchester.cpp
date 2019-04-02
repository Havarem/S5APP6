#include "manchester.h"
#include "mbed.h"

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

void to_manchester(char res[2], char in)
{
  res[0] = 0;
  res[1] = 0;
  for(int i = 0; i < 4; i++)
  {
    char bit = (in >> i) & 0x01;
    res[0] = res[0] | !bit << 2*i;
    res[0] = res[0] | bit << (2*i+1); 
    //printf("%d %d %d\r\n", bit,  (res[0] >> 2*i) & 0x01, (res[0] >> (2*i + 1)) & 0x01);
  }
  for(int i = 4; i < 8; i++)
  {
    char bit = (in >> i) & 0x01;
    res[1] = res[1] | !bit << 2*(i-4);
    res[1] = res[1] | bit << (2*(i-4) + 1); 
    //printf("%d %d %d\r\n", bit,  (res[1] >> 2*i) & 0x01, (res[1] >> (2*i + 1)) & 0x01);
  }
}