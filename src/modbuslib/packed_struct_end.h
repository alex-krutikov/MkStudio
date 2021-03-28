#if defined(Q_CC_MSVC)

    #pragma pack(pop)

#elif defined(Q_CC_GNU)
#else

    #error Unsupported compiler

#endif
