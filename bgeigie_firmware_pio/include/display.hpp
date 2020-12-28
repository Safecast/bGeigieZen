#ifndef __DISPLAY_H__
#define __DISPLAY_H__

void printFloat(float val, bool valid, int len, int prec);
void printFloatFont(float val, bool valid, int len, int prec, int y, int x, int font);
void printInt(unsigned long val, bool valid, int len);
void printIntFont(unsigned long val, bool valid, int len, int y, int x, int font);
//void printDate(TinyGPSDate &d)
//void printTime(TinyGPSTime &t)

#endif // __DISPLAY_H__
