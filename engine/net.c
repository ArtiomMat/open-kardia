#include "net.h"

net_addr_t net_host_addr;
const net_addr_t net_loopback = {.b[15]=1};
