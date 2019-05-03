/*
 * This class should wrap around hardware serial interfaces.
 * By having such a class, we can create virtual devices for simulation
 */

#ifndef __SERIAL_SOURCE_H__
#define __SERIAL_SOURCE_H__

#include <cstdio>

class SerialSource
{
  public:
    virtual bool available() = 0;
    virtual char read() = 0;
};

class SerialSink
{
  public:
    virtual size_t println(char *buf) = 0;
    virtual size_t write(char *buf) = 0;
};

#endif
