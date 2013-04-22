//************************************************************************
//
//      Open Navigation Surface Working Group, 2013
//
//************************************************************************
#ifndef BAG_CONFIG_H
#define BAG_CONFIG_H

#if defined(BAG_DLL) && defined(_MSC_VER)

    #ifdef __cplusplus
        #ifdef BAG_EXPORTS
            #define BAG_EXTERNAL extern "C" __declspec(dllexport)
        #else
            #define BAG_EXTERNAL extern "C" __declspec(dllimport)
        #endif
    #else
        #ifdef BAG_EXPORTS
            #define BAG_EXTERNAL extern __declspec(dllexport)
        #else
            #define BAG_EXTERNAL extern __declspec(dllimport)
        #endif
    #endif

#else

    #ifdef __cplusplus
        #define BAG_EXTERNAL extern "C"
    #else
        #define BAG_EXTERNAL extern
    #endif
    
#endif /* BAG_DLL && _MSC_VER */

#endif