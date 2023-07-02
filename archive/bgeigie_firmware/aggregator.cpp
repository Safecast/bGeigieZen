#include "aggregator.hpp"


void Aggregator::update()
{
  if (_gps->available())
  {
    bool got_data = _gps->get_data(_current_data.gps);
  }

}
