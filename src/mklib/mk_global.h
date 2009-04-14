#ifndef __MKLIB_GLOBAL__H__
#define __MKLIB_GLOBAL__H__

#ifdef Q_OS_WIN32
  #ifdef MKLIB_DLL
//    #define MK_EXPORT __declspec(dllexport)
  #else
//    #define MK_EXPORT __declspec(dllimport)
  #endif
#endif

#ifndef MK_EXPORT
  #define MK_EXPORT
#endif

#endif
