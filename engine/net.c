#include "net.h"

net_addr_t net_host_addr;
const net_addr_t net_loopback = {.b[15]=1};

#define PAD(WHAT, P) \
  {\
    int __off = WHAT % P;\
    if (__off)\
    {\
      WHAT += (1<<P) - __off;\
    }\
  }

int
net_put16(net_sock_t* s, uint16_t x)
{
  PAD(s->pout.cur, 2);

  *(uint16_t*)(s->pout.data + s->pout.cur) = com_lil16(x);
  s->pout.cur += 2;
  return NET_MAX_PACK_SIZE - s->pout.cur;
}
int
net_put32(net_sock_t* s, uint32_t x)
{
  PAD(s->pout.cur, 4);

  *(uint32_t*)(s->pout.data + s->pout.cur) = com_lil32(x);
  s->pout.cur += 4;
  return NET_MAX_PACK_SIZE - s->pout.cur;
}
int
net_puts(net_sock_t* s, const char* str)
{
  do
  {
    s->pout.data[s->pout.cur++] = *str;
    if (s->pout.cur >= NET_MAX_PACK_SIZE)
    {
      // s->pout.data[s->pout.cur-1] = 0; // Null terminate
      return -1;
    }
    str++;
  } while (*str);

  s->pout.data[s->pout.cur++] = 0;

  return NET_MAX_PACK_SIZE - s->pout.cur;
}
int
net_putb(net_sock_t* s, const char* data, int n)
{
  for (int i = 0; i < n; i++)
  {
    s->pout.data[s->pout.cur++] = data[i];
  }
  return NET_MAX_PACK_SIZE - s->pout.cur;
}

int
net_get16(net_sock_t* s, uint16_t* x)
{
  PAD(s->pin.cur, 2);

  *x = com_lil16(*(uint16_t*)(s->pin.data + s->pin.cur));
  s->pin.cur += 2;
  return s->pin.size - s->pin.cur;
}
int
net_get32(net_sock_t* s, uint32_t* x)
{
  PAD(s->pin.cur, 4);

  *x = com_lil32(*(uint32_t*)(s->pin.data + s->pin.cur));
  s->pin.cur += 4;
  return s->pin.size - s->pin.cur;
}
int
net_gets(net_sock_t* s, const char** str)
{
  *str = s->pin.data + s->pin.cur;

  for (; s->pin.data[s->pin.cur]; s->pin.cur++)
  {
    // If at last character, it means we overflowed the string, null terminate.
    if (s->pin.cur >= s->pin.size - 1)
    {
      s->pin.data[s->pin.cur] = 0;
      return -1;
    }
  }

  s->pin.cur++;

  return s->pin.size - s->pin.cur;
}
int
net_getb(net_sock_t* s, const char** data, int n)
{
  *data = s->pin.data + s->pin.cur;
  s->pin.cur += n;
  return s->pin.size - s->pin.cur;
}
