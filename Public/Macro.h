#ifndef _STONE_MACRO_H_
#define _STONE_MACRO_H_

#undef LIKELY
#undef UNLIKELY

#if defined(__GNUC__) && __GNUC__ >= 4
#define LIKELY(x)   (__builtin_expect(!!(x), 1))
#define UNLIKELY(x) (__builtin_expect(!!(x), 0))
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

// An attribute that will cause a variable or field to be aligned so that
// it doesn't have false sharing with anything at a smaller memory address.
// the value of a cache line need to be confirmed by test
#define SIZE_OF_AVOID_FALSE_SHARING 128

#define DEFAULT_QUEUE_BUFFER_SIZE 256

#define ALIGN_TO_AVOID_FALSE_SHARING __attribute__((__aligned__(SIZE_OF_AVOID_FALSE_SHARING)))



/*************************Log******************************/
#include <string>
namespace Stone
{
	std::string GetThreadName(void);
	std::string GetCurrentTime(void);
}

#include <libgen.h>
#include <stdio.h>
#define _PRI(fmt, args...)  printf("\033[34m [%s][%s][%s %s:%d]" fmt "\033[0m\n", Stone::GetCurrentTime().c_str(), Stone::GetThreadName().c_str(), basename((char*)__FILE__), __func__, __LINE__, ##args)
#define _ERR(fmt, args...)  printf("\033[31m [%s][%s][%s %s:%d]" fmt "\033[0m\n", Stone::GetCurrentTime().c_str(), Stone::GetThreadName().c_str(), basename((char*)__FILE__), __func__, __LINE__, ##args)
#define _WRN(fmt, args...)  printf("\033[33m [%s][%s][%s %s:%d]" fmt "\033[0m\n", Stone::GetCurrentTime().c_str(), Stone::GetThreadName().c_str(), basename((char*)__FILE__), __func__, __LINE__, ##args)
#define _DBG(fmt, args...)  printf("\033[32m [%s][%s][%s %s:%d]" fmt "\033[0m\n", Stone::GetCurrentTime().c_str(), Stone::GetThreadName().c_str(), basename((char*)__FILE__), __func__, __LINE__, ##args)
#define _INFO(fmt, args...) printf("\033[37m [%s][%s][%s %s:%d]" fmt "\033[0m\n", Stone::GetCurrentTime().c_str(), Stone::GetThreadName().c_str(), basename((char*)__FILE__), __func__, __LINE__, ##args)



#endif


