#ifndef __HIRES_SLEEP_H__
#define __HIRES_SLEEP_H__

#include <windows.h>

class HiResSleep
{
private:
  LARGE_INTEGER freq;
  BOOL PerfCntOk;
public:
  HiResSleep() ;
  void UsSleep(DWORD Us) ;
};

#endif
