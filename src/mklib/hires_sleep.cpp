
#include "hires_sleep.h"

HiResSleep::HiResSleep()
{
  if (!(PerfCntOk = QueryPerformanceCounter(&freq))) return;
  PerfCntOk = QueryPerformanceFrequency(&freq);
}

void HiResSleep::UsSleep(DWORD Us)
{
  LARGE_INTEGER curr, stop;

  if (PerfCntOk)
  {
    QueryPerformanceCounter(&curr);
    stop.QuadPart = curr.QuadPart + (freq.QuadPart * 1000 * Us / 1000000000);
    do
    {
      Sleep(0);
      QueryPerformanceCounter(&curr);
    } while (curr.QuadPart < stop.QuadPart);

    return;
  }
  Sleep((Us + 999) / 1000);
}
