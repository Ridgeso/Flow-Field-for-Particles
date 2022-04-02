#ifndef __LOGGING_H__
#define __LOGGING_H__


#include "internal.h"

void clearError();
uint32_t checError(const char* func);

#define LOG(x, y) printf(__TIME__" | "__FILE__" | LINE:%d | ["#x"]["#y"]\n", __LINE__);

#if defined(DEBUG) && (defined(__x86_64__) || defined(__i386__))
    __inline__ static void __debugbreak() {
        __asm__ volatile("int $0x03");
    }
    #define ASSERT(x, y) if (x) { LOG(x, y); __debugbreak(); }
    #define GLCall(x) \
        clearError();\
        x;\
        ASSERT(!checError(#x),)    
    #define WARNING(x, y) if(x) { LOG(x, y) }
#else
    #define ASSERT(x, y) if(x) { LOG(x, y); exit(1); }
    #define WARNING(x, y)
    #define GLCall(x) x
#endif

#endif // __LOGGING_H__