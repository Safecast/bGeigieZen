#ifndef __MEASUREMENT_H__
#define __MEASUREMENT_H__

#include <string>
#include "patterns.hpp"

using std::string;

struct MeasurementData
{
  bool fresh = false;
};

template<class T>
class Measurement : public Subject
{
  private:
    bool _available = false;

  protected:
    // set state to ready
    void set_avalaible() { _available = true; }
    void unset_avalaible() { _available = false; }

  public:
    virtual void init() = 0;
    virtual bool available() { return _available; }

    // This method actually copy the data
    // The derived to should make sure there
    // that everything is ready
    virtual bool prepare_data(T &dest) = 0;

    // This method returns the data and mark the
    // data as consumed (set ready to false)
    bool get_data(T &dest)
    {
      if (prepare_data(dest))
      {
        _available = false;
        return true;
      }

      return false;
    }

};

#endif // __MEASUREMENT_H__
