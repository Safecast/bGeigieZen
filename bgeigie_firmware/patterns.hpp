#ifndef __PATTERNS_H__
#define __PATTERNS_H__

#include <forward_list>

class Observer
{
  public:
    virtual void update() = 0;
};

class Subject
{
  private:
    std::forward_list<Observer *> observers;

  public:
    void register_observer(Observer *obs) {
      observers.push_front(obs);
    }
};

#endif // __PATTERNS_H__
