#include "sock.hpp"

namespace axe
{
  addr_t host_addr = {0};
  const char* host_name = nullptr;

  int sock_t::put16(uint16_t x)
  {
    _NET_PAD(this->pout.cur, 2);

    *(uint16_t*)(this->pout.data + this->pout.cur) = lil16(x);
    this->pout.cur += 2;
    return MAX_PACK_SIZE - this->pout.cur;
  }
  int sock_t::put32(uint32_t x)
  {
    _NET_PAD(this->pout.cur, 4);

    *(uint32_t*)(this->pout.data + this->pout.cur) = lil32(x);
    this->pout.cur += 4;
    return MAX_PACK_SIZE - this->pout.cur;
  }
  int sock_t::puts(const char* str)
  {
    do
    {
      this->pout.data[this->pout.cur++] = *str;
      if (this->pout.cur >= MAX_PACK_SIZE)
      {
        this->pout.data[MAX_PACK_SIZE-1] = 0; // Null terminate
        return -1;
      }
      str++;
    } while (*str);

    this->pout.data[this->pout.cur++] = 0;

    return MAX_PACK_SIZE - this->pout.cur;
  }
  int sock_t::putb(const char* data, int n)
  {
    for (int i = 0; i < n; i++)
    {
      this->pout.data[this->pout.cur++] = data[i];
    }
    return MAX_PACK_SIZE - this->pout.cur;
  }

  uint16_t sock_t::get16()
  {
    _NET_PAD(this->pin.cur, 2);

    uint16_t x = lil16(*(uint16_t*)(this->pin.data + this->pin.cur));
    this->pin.cur += 2;
    return x;
  }
  uint32_t sock_t::get32()
  {
    _NET_PAD(this->pin.cur, 4);

    uint32_t x = lil32(*(uint32_t*)(this->pin.data + this->pin.cur));
    this->pin.cur += 4;
    return x;
  }
  const char* sock_t::gets()
  {
    const char* str = this->pin.data + this->pin.cur;

    for (; this->pin.data[this->pin.cur]; this->pin.cur++)
    {
      // If at last character, it means we overflowed the string, null terminate.
      if (this->pin.cur >= this->pin.size - 1)
      {
        this->pin.data[this->pin.cur] = 0;
        return nullptr;
      }
    }

    this->pin.cur++;

    return str;
  }
  const char* sock_t::getb(int n)
  {
    const char* data = this->pin.data + this->pin.cur;
    this->pin.cur += n;
    return data;
  }

  unsigned sock_t::gets_n()
  {
    unsigned i;

    for (i = this->pin.cur; this->pin.data[i] && i < this->pin.size; i++)
    {}
    
    return i - this->pin.cur + 1;
  }
}
