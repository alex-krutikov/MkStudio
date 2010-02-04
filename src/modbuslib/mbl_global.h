#ifndef __MBL_GLOBAL__H__
#define __MBL_GLOBAL__H__

#ifdef Q_OS_WIN32
  #ifdef MBL_DLL
    //#define MBL_EXPORT __declspec(dllexport)
  #else
    //#define MBL_EXPORT __declspec(dllimport)
  #endif
#endif

#ifndef MBL_EXPORT
  #define MBL_EXPORT
#endif

//#if defined(Q_CC_MSVC)

#define PACKED_STRUCT_BEGIN   __pragma pack(push, 1)

#define PACKED_STRUCT_HEADER

#define PACKED_STRUCT_END    __pragma pack(pop)

#endif
