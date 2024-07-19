#include "net.hpp"

namespace net
{
  addr_t host_addr = {0};
  const char* host_name = "NET UNITIALIZED";

  int sock_t::put16(uint16_t x)
  {
    NET_PAD(this->pout.cur, 2);

    *(uint16_t*)(this->pout.data + this->pout.cur) = com::lil16(x);
    this->pout.cur += 2;
    return MAX_PACK_SIZE - this->pout.cur;
  }
  int sock_t::put32(uint32_t x)
  {
    NET_PAD(this->pout.cur, 4);

    *(uint32_t*)(this->pout.data + this->pout.cur) = com::lil32(x);
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

  int sock_t::get16(uint16_t& x)
  {
    NET_PAD(this->pin.cur, 2);

    x = com::lil16(*(uint16_t*)(this->pin.data + this->pin.cur));
    this->pin.cur += 2;
    return this->pin.size - this->pin.cur;
  }
  int sock_t::get32(uint32_t& x)
  {
    NET_PAD(this->pin.cur, 4);

    x = com::lil32(*(uint32_t*)(this->pin.data + this->pin.cur));
    this->pin.cur += 4;
    return this->pin.size - this->pin.cur;
  }
  int sock_t::gets(const char*& str)
  {
    str = this->pin.data + this->pin.cur;

    for (; this->pin.data[this->pin.cur]; this->pin.cur++)
    {
      // If at last character, it means we overflowed the string, null terminate.
      if (this->pin.cur >= this->pin.size - 1)
      {
        this->pin.data[this->pin.cur] = 0;
        return -1;
      }
    }

    this->pin.cur++;

    return this->pin.size - this->pin.cur;
  }
  int sock_t::getb(const char*& data, int n)
  {
    data = this->pin.data + this->pin.cur;
    this->pin.cur += n;
    return this->pin.size - this->pin.cur;
  }

  int sock_t::gets_n()
  {
    int i;

    for (i = this->pin.cur; this->pin.data[i] && i < this->pin.size; i++)
    {}
    
    return i - this->pin.cur + 1;
  }
}
