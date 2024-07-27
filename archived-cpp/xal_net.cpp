#include "xal.hpp"

namespace xal
{
  void server_t::handler(net::server_t::event_t& e)
  {
    if (e.type == net::E_DISJOIN)
    {
      puts("Client disjoined");
    }
  }

  void client_t::handler(net::client_t::event_t& e)
  {
    
  }
}
