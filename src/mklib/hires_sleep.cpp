
#include "hires_sleep.h"

HiResSleep::HiResSleep()
{
  if (QueryPerformanceFrequency(&freq)) PerfCntOk = 1 ;
  else PerfCntOk = 0 ;
}

void HiResSleep::UsSleep(DWORD Us)
{
  LARGE_INTEGER curr, stop;

  do
  {
    if (PerfCntOk)
    {
      if (!QueryPerformanceCounter(&curr))
      {
        PerfCntOk = 0;
        break;
      }

      stop.QuadPart = curr.QuadPart + (freq.QuadPart * 1000 * Us / 1000000000);

      do
      {
        Sleep(0);
        QueryPerformanceCounter(&curr);
      } while (curr.QuadPart < stop.QuadPart);

      return;
    }
  } while (0);

  Sleep((Us + 999) / 1000);

}

