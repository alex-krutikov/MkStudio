#if defined(Q_CC_MSVC)

  #pragma warning(disable: 4103)
  #pragma pack(push, 1)

#elif defined(Q_CC_GNU)

  #ifndef PACKED_STRUCT_HEADER
  #define PACKED_STRUCT_HEADER __attribute__((packed))
  #endif

#else

  #error Unsupported compiler

#endif
