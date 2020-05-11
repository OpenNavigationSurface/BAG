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
            #define BAG_API __declspec(dllexport)
        #else
            #define BAG_EXTERNAL extern "C" __declspec(dllimport)
            #define BAG_API __declspec(dllimport)
        #endif
    #else
        #ifdef BAG_EXPORTS
            #define BAG_EXTERNAL extern __declspec(dllexport)
            #define BAG_API
        #else
            #define BAG_EXTERNAL extern __declspec(dllimport)
            #define BAG_API
        #endif
    #endif
#else
    #ifdef __cplusplus
        #define BAG_EXTERNAL extern "C"
        #define BAG_API
    #else
        #define BAG_EXTERNAL extern
        #define BAG_API
    #endif
#endif  // BAG_DLL && _MSC_VER

#endif  // BAG_CONFIG_H

