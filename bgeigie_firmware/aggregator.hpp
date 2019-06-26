#ifndef __AGGREGATOR_H__
#define __AGGREGATOR_H__

#include <forward_list>
#include "threaded_gps.hpp"
#include "patterns.hpp"
#include "measurements.hpp"

using std::forward_list;

struct DataStamp
{
  GPSData gps;

  // TBD
  // RadiationData rad;
};

class Aggregator : public Observer
{
  private:
    DataStamp _current_data;

  public:
    Aggregator();
    virtual ~Aggregator();

    // method to be called by subjects
    void update();
};

#endif // __AGGREGATOR_H__
