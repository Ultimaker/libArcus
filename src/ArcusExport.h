#ifndef ARCUS_EXPORT_H
#define ARCUS_EXPORT_H

#if _WIN32
    #ifdef MAKE_ARCUS_LIB
        #define ARCUS_EXPORT __declspec(dllexport)
    #else
        #define ARCUS_EXPORT
    #endif
#else
    #define ARCUS_EXPORT
#endif

#endif
